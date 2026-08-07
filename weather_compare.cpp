// weather_compare.cpp
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <chrono>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

struct WeatherData {
    string city;
    string temp;
    string humidity;
    string wind;
    string pressure;
    string cloud;
    string desc;
};

struct CacheEntry {
    WeatherData data;
    chrono::steady_clock::time_point time;
};

map<string, CacheEntry> cache;
const int CACHE_TTL = 300; // seconds

size_t WriteCallback(void *contents, size_t size, size_t nmemb, string *output) {
    size_t total = size * nmemb;
    output->append((char*)contents, total);
    return total;
}

string fetchUrl(const string& url) {
    CURL* curl = curl_easy_init();
    string response;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        if (res != CURLE_OK) {
            return "";
        }
    }
    return response;
}

WeatherData getWeather(const string& city) {
    // Cache
    auto now = chrono::steady_clock::now();
    auto it = cache.find(city);
    if (it != cache.end() && chrono::duration_cast<chrono::seconds>(now - it->second.time).count() < CACHE_TTL) {
        return it->second.data;
    }

    string url = "https://wttr.in/" + city + "?format=j1&lang=ru";
    string response = fetchUrl(url);
    if (response.empty()) {
        return WeatherData{city, "N/A", "N/A", "N/A", "N/A", "N/A", ""};
    }
    try {
        json j = json::parse(response);
        auto& current = j["current_condition"][0];
        WeatherData w;
        w.city = city;
        w.temp = current.value("temp_C", "N/A");
        w.humidity = current.value("humidity", "N/A");
        w.wind = current.value("windSpeed", "N/A");
        w.pressure = current.value("pressure", "N/A");
        w.cloud = current.value("cloudcover", "N/A");
        w.desc = current["weatherDesc"][0].value("value", "");
        cache[city] = {w, now};
        return w;
    } catch (...) {
        return WeatherData{city, "N/A", "N/A", "N/A", "N/A", "N/A", "Ошибка"};
    }
}

string center(const string& s, int width) {
    if ((int)s.length() >= width) return s.substr(0, width);
    int left = (width - s.length()) / 2;
    int right = width - s.length() - left;
    return string(left, ' ') + s + string(right, ' ');
}

void printSeparator(const vector<int>& widths) {
    cout << "+";
    for (int w : widths) cout << string(w, '-') << "+";
    cout << endl;
}

void printRow(const vector<string>& row, const vector<int>& widths) {
    cout << "|";
    for (size_t i=0; i<row.size(); i++) {
        cout << center(row[i], widths[i]) << "|";
    }
    cout << endl;
}

void printTable(const vector<WeatherData>& list) {
    if (list.empty()) {
        cout << "Нет данных для отображения." << endl;
        return;
    }
    vector<string> headers = {"Город", "Темп.", "Влаж.", "Ветер", "Давл.", "Облач.", "Описание"};
    vector<int> widths = {15, 8, 8, 8, 8, 8, 20};
    printSeparator(widths);
    printRow(headers, widths);
    printSeparator(widths);
    for (const auto& w : list) {
        vector<string> row = {
            center(w.city, widths[0]),
            center(w.temp + "°C", widths[1]),
            center(w.humidity + "%", widths[2]),
            center(w.wind + " км/ч", widths[3]),
            center(w.pressure + " мм", widths[4]),
            center(w.cloud + "%", widths[5]),
            center(w.desc, widths[6])
        };
        printRow(row, widths);
    }
    printSeparator(widths);
}

void interactive() {
    vector<string> cities;
    cout << "Weather Compare (интерактивный режим)" << endl;
    cout << "Команды: add <город>, remove <город>, show, save, load, quit" << endl;
    string line;
    while (true) {
        cout << "> ";
        getline(cin, line);
        if (line.empty()) continue;
        vector<string> parts;
        string word;
        for (char c : line) {
            if (c == ' ') {
                if (!word.empty()) { parts.push_back(word); word.clear(); }
            } else word += c;
        }
        if (!word.empty()) parts.push_back(word);
        if (parts.empty()) continue;
        if (parts[0] == "quit") break;
        else if (parts[0] == "add" && parts.size() > 1) {
            string city = line.substr(parts[0].size()+1);
            if (find(cities.begin(), cities.end(), city) == cities.end()) {
                cities.push_back(city);
                cout << "Город " << city << " добавлен." << endl;
            } else {
                cout << "Город " << city << " уже есть." << endl;
            }
        }
        else if (parts[0] == "remove" && parts.size() > 1) {
            string city = line.substr(parts[0].size()+1);
            auto it = find(cities.begin(), cities.end(), city);
            if (it != cities.end()) {
                cities.erase(it);
                cout << "Город " << city << " удалён." << endl;
            } else {
                cout << "Город " << city << " не найден." << endl;
            }
        }
        else if (parts[0] == "show") {
            if (cities.empty()) {
                cout << "Список городов пуст." << endl;
                continue;
            }
            vector<WeatherData> weatherList;
            for (const string& city : cities) {
                WeatherData w = getWeather(city);
                weatherList.push_back(w);
            }
            printTable(weatherList);
        }
        else if (parts[0] == "save") {
            ofstream f("favorites.txt");
            if (f.is_open()) {
                for (const string& c : cities) f << c << endl;
                f.close();
                cout << "Список сохранён в favorites.txt" << endl;
            }
        }
        else if (parts[0] == "load") {
            ifstream f("favorites.txt");
            if (f.is_open()) {
                cities.clear();
                string c;
                while (getline(f, c)) {
                    if (!c.empty()) cities.push_back(c);
                }
                cout << "Список загружен из favorites.txt" << endl;
            } else {
                cout << "Файл favorites.txt не найден." << endl;
            }
        }
        else {
            cout << "Неизвестная команда." << endl;
        }
    }
}

int main(int argc, char** argv) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    if (argc == 1) {
        interactive();
    } else {
        vector<WeatherData> weatherList;
        for (int i=1; i<argc; i++) {
            string city = argv[i];
            weatherList.push_back(getWeather(city));
        }
        printTable(weatherList);
    }
    curl_global_cleanup();
    return 0;
}
