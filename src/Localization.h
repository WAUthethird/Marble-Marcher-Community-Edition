#pragma once
#include <map>
#include <algorithm>
#include <SFML/Graphics.hpp>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <Level.h>
#include <locale>
#include <Res.h>
#include <codecvt>

namespace fs = std::filesystem;

class Localization
{
public:
	Localization();

	void LoadLocalsFromFolder(std::string folder);
	void LoadLocalFromFile(fs::path path);
	void SetLanguage(std::string lang);
	std::vector<std::string> GetLanguages();

	std::wstring operator[](std::string str);
	std::string str(std::string str);
	const char * cstr(std::string str);
	sf::Font& operator()(std::string str);

	void del();
private:
	std::string cur_language;
	std::vector<std::string> languages;
	std::map<std::string, std::map<std::string, std::wstring>> locales;
	std::map<std::string, std::map<std::string, sf::Font>> fonts;

	std::unique_ptr<sf::Font> default_font;
};

extern Localization LOCAL;

std::wstring utf8_to_wstring(const std::string& str);