#pragma once
//Offline beat analysis for the level music with a 5 band sensitivity equalizer.
//The track is decoded once in a background thread into 100 Hz per-band onset
//peaks; the beat value for a playback position + equalizer setting is then a
//cheap lookup, so the equalizer can be changed live in the level editor.
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <atomic>
#include <thread>

static const int BEAT_BANDS = 5;
//band names for the UI
static const char* const BEAT_BAND_NAMES[BEAT_BANDS] = {
	"Sub bass (<120 Hz)", "Bass (120-400 Hz)", "Mids (0.4-2 kHz)", "High mids (2-6 kHz)", "Highs (>6 kHz)" };

class MusicAnalyzer
{
public:
	static const int RATE = 100; //envelope samples per second

	~MusicAnalyzer();

	//start analysing a track in the background (no-op if already known)
	void Request(const std::string& name, const std::string& path);

	//beat pulse 0..1 (1 right on a beat, decays quickly)
	//eq: per band sensitivity, 0 = band ignored, 1 = normal, 3 = very sensitive
	float GetBeat(const std::string& name, float time_sec, const float eq[BEAT_BANDS]);
	//smoothed loudness 0..1
	float GetEnergy(const std::string& name, float time_sec);

private:
	struct Envelope
	{
		//onset peak ratio (energy / local average) at local maxima, 0 elsewhere
		std::vector<float> peaks[BEAT_BANDS];
		std::vector<float> energy;
		std::atomic<bool> ready{ false };
	};

	static void Analyze(std::shared_ptr<Envelope> env, std::string path);
	std::shared_ptr<Envelope> Find(const std::string& name);

	std::mutex mtx;
	std::map<std::string, std::shared_ptr<Envelope>> envelopes;
	std::vector<std::thread> workers;
};

extern MusicAnalyzer MUSIC_ANALYZER;
