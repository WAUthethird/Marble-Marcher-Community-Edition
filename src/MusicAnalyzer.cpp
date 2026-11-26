#include "MusicAnalyzer.h"
#include <SFML/Audio.hpp>
#include <cmath>
#include <algorithm>

MusicAnalyzer MUSIC_ANALYZER;

MusicAnalyzer::~MusicAnalyzer()
{
	for (auto& t : workers)
	{
		if (t.joinable()) t.join();
	}
}

void MusicAnalyzer::Request(const std::string& name, const std::string& path)
{
	if (name.empty() || path.empty()) return;
	std::lock_guard<std::mutex> lock(mtx);
	if (envelopes.count(name) > 0) return;
	auto env = std::make_shared<Envelope>();
	envelopes[name] = env;
	workers.emplace_back(&MusicAnalyzer::Analyze, env, path);
}

void MusicAnalyzer::Analyze(std::shared_ptr<Envelope> env, std::string path)
{
	sf::InputSoundFile file;
	if (!file.openFromFile(path))
	{
		env->ready = true; //empty envelope => no reaction
		return;
	}

	const unsigned int sr = file.getSampleRate();
	const unsigned int ch = std::max(1u, file.getChannelCount());
	const unsigned int hop = std::max(1u, sr / RATE);

	//crossover frequencies of the band split (cascaded one pole low passes)
	const float cut[BEAT_BANDS - 1] = { 120.f, 400.f, 2000.f, 6000.f };
	float a[BEAT_BANDS - 1], lp[BEAT_BANDS - 1] = { 0, 0, 0, 0 };
	for (int b = 0; b < BEAT_BANDS - 1; b++)
		a[b] = std::exp(-2.0f * 3.14159265f * cut[b] / float(sr));

	std::vector<float> e_band[BEAT_BANDS], e_full;
	double acc[BEAT_BANDS] = { 0 }, acc_f = 0;
	std::vector<sf::Int16> buffer(hop * ch * 64);
	unsigned int count = 0;

	while (true)
	{
		sf::Uint64 read = file.read(buffer.data(), buffer.size());
		if (read == 0) break;
		for (sf::Uint64 i = 0; i + ch <= read; i += ch)
		{
			float s = 0;
			for (unsigned int c = 0; c < ch; c++) s += buffer[i + c];
			s /= (32768.f * ch);
			for (int b = 0; b < BEAT_BANDS - 1; b++)
				lp[b] = a[b] * lp[b] + (1.f - a[b]) * s;
			float band[BEAT_BANDS] = { lp[0], lp[1] - lp[0], lp[2] - lp[1], lp[3] - lp[2], s - lp[3] };
			for (int b = 0; b < BEAT_BANDS; b++) acc[b] += band[b] * band[b];
			acc_f += s * s;
			if (++count == hop)
			{
				for (int b = 0; b < BEAT_BANDS; b++) { e_band[b].push_back(float(acc[b] / hop)); acc[b] = 0; }
				e_full.push_back(float(acc_f / hop));
				acc_f = 0;
				count = 0;
			}
		}
	}

	const int n = (int)e_full.size();
	const int win = RATE / 2;       //local average over +-0.5 s
	const int peak_win = RATE / 16; //local maximum over +-60 ms
	for (int b = 0; b < BEAT_BANDS; b++)
	{
		std::vector<float> peaks(n, 0.f);
		const std::vector<float>& e = e_band[b];
		std::vector<double> pre(n + 1, 0.0);
		for (int i = 0; i < n; i++) pre[i + 1] = pre[i] + e[i];
		//ignore near-silent passages of this band
		double total = pre[n] / std::max(n, 1);
		for (int i = 0; i < n; i++)
		{
			if (e[i] < 0.02 * total || e[i] < 1e-9f) continue;
			bool is_peak = true;
			for (int j = std::max(0, i - peak_win); j < std::min(n, i + peak_win + 1); j++)
			{
				if (e[j] > e[i]) { is_peak = false; break; }
			}
			if (!is_peak) continue;
			int lo = std::max(0, i - win), hi = std::min(n, i + win + 1);
			float avg = float((pre[hi] - pre[lo]) / (hi - lo));
			peaks[i] = e[i] / (avg + 1e-12f);
		}
		env->peaks[b].swap(peaks);
	}

	//loudness envelope, normalized by the 95th percentile
	std::vector<float> energy(n, 0.f);
	if (n > 0)
	{
		std::vector<float> rms(n);
		for (int i = 0; i < n; i++) rms[i] = std::sqrt(e_full[i]);
		std::vector<float> sorted = rms;
		std::nth_element(sorted.begin(), sorted.begin() + (n * 95) / 100, sorted.end());
		float norm = std::max(sorted[(n * 95) / 100], 1e-5f);
		float sm = 0;
		for (int i = 0; i < n; i++)
		{
			sm = 0.8f * sm + 0.2f * rms[i];
			energy[i] = std::min(1.f, sm / norm);
		}
	}
	env->energy.swap(energy);
	env->ready = true;
}

std::shared_ptr<MusicAnalyzer::Envelope> MusicAnalyzer::Find(const std::string& name)
{
	std::lock_guard<std::mutex> lock(mtx);
	auto it = envelopes.find(name);
	if (it == envelopes.end() || !it->second->ready) return nullptr;
	return it->second;
}

float MusicAnalyzer::GetBeat(const std::string& name, float time_sec, const float eq[BEAT_BANDS])
{
	std::shared_ptr<Envelope> env = Find(name);
	if (!env) return 0.f;
	const int n = (int)env->peaks[0].size();
	if (n == 0) return 0.f;

	//small look-ahead to compensate the audio output latency
	const int now = std::min(int((time_sec + 0.03f) * RATE), n - 1);
	const int lookback = RATE * 3 / 5;       //a pulse lasts ~0.6 s
	const float decay = 0.14f * RATE;        //decay time constant

	//per band threshold: higher sensitivity => lower threshold
	float thr[BEAT_BANDS];
	for (int b = 0; b < BEAT_BANDS; b++)
		thr[b] = (eq[b] > 0.001f) ? 1.0f + 0.6f / eq[b] : 1e30f;

	float beat = 0.f;
	for (int k = 0; k <= lookback && now - k >= 0; k++)
	{
		const float fade = std::exp(-float(k) / decay);
		if (fade <= beat) break; //older beats can not be stronger anymore
		const int i = now - k;
		for (int b = 0; b < BEAT_BANDS; b++)
		{
			const float p = env->peaks[b][i];
			if (p > thr[b])
			{
				const float strength = std::min(1.f, 0.55f + 0.45f * (p - thr[b]) / thr[b] * 2.f);
				beat = std::max(beat, strength * fade);
			}
		}
	}
	return beat;
}

float MusicAnalyzer::GetEnergy(const std::string& name, float time_sec)
{
	std::shared_ptr<Envelope> env = Find(name);
	if (!env || env->energy.empty()) return 0.f;
	int i = std::max(0, std::min(int((time_sec + 0.03f) * RATE), int(env->energy.size()) - 1));
	return env->energy[i];
}
