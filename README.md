🌤️ Weather Compare — сравнение погоды в городах
Версия: 1.0.0 | Лицензия: MIT | Статус: ✅ Активная разработка

https://img.shields.io/github/repo-size/yourusername/weather-compare https://img.shields.io/github/last-commit/yourusername/weather-compare https://img.shields.io/github/languages/count/yourusername/weather-compare

☀️ Описание
Weather Compare – это консольная утилита для сравнения текущей погоды в нескольких городах одновременно. Программа получает данные с бесплатного API wttr.in и отображает их в удобной таблице: температура, влажность, скорость ветра, давление и облачность.

Возможности:

✅ Сравнение погоды для произвольного количества городов

✅ Отображение температуры, влажности, ветра, давления, облачности

✅ Цветовая индикация температуры (холодно/тепло/жарко)

✅ Сохранение списка любимых городов в файл

✅ Загрузка списка городов из файла

✅ Поддержка интернациональных названий (на русском и английском)

✅ Кеширование запросов на 5 минут для экономии трафика

✅ Интерактивный режим с меню

Проект содержит 8 полноценных реализаций на разных языках программирования. Все версии используют консольный вывод и минимальные зависимости (только HTTP-клиент и JSON-парсер).

🖥️ Скриншоты
(В реальном репозитории замените на свои изображения)

https://via.placeholder.com/800x400?text=%D0%A1%D1%80%D0%B0%D0%B2%D0%BD%D0%B5%D0%BD%D0%B8%D0%B5+%D0%BF%D0%BE%D0%B3%D0%BE%D0%B4%D1%8B
Таблица с погодой для четырёх городов.

https://via.placeholder.com/800x400?text=%D0%98%D0%BD%D1%82%D0%B5%D1%80%D0%B0%D0%BA%D1%82%D0%B8%D0%B2%D0%BD%D1%8B%D0%B9+%D1%80%D0%B5%D0%B6%D0%B8%D0%BC
Меню для добавления городов и просмотра.

📦 Установка и запуск
Каждая реализация находится в отдельной папке. Для запуска требуется соответствующий компилятор/интерпретатор и зависимости.

Язык	Файл	Зависимости	Команда запуска
Python	weather_compare.py	requests (опционально)	pip install requests && python3 weather_compare.py
Go	weather_compare.go	нет (встроенный net/http)	go run weather_compare.go
Rust	weather_compare.rs	reqwest, serde_json	cargo run
C++	weather_compare.cpp	libcurl, nlohmann/json	g++ -std=c++17 -lcurl -o weather weather_compare.cpp && ./weather
Java	WeatherCompare.java	java.net.http (Java 11+)	javac WeatherCompare.java && java WeatherCompare
C#	weather_compare.cs	Newtonsoft.Json	dotnet add package Newtonsoft.Json && dotnet run
Ruby	weather_compare.rb	json, net/http (встроен)	ruby weather_compare.rb
Node.js	weather_compare.js	axios (или node-fetch)	npm install axios && node weather_compare.js
Примечание: Для работы требуется интернет-соединение. Все версии используют публичный API wttr.in, который не требует ключа и имеет ограничение ~1 запрос в секунду.

📂 Структура репозитория
text
.
├── README.md
├── python/
│   └── weather_compare.py
├── go/
│   └── weather_compare.go
├── rust/
│   ├── Cargo.toml
│   └── src/
│       └── main.rs
├── cpp/
│   └── weather_compare.cpp
├── java/
│   └── WeatherCompare.java
├── csharp/
│   └── weather_compare.cs
├── ruby/
│   └── weather_compare.rb
└── javascript/
    ├── package.json
    └── weather_compare.js
🛠️ Особенности реализаций
Python – использует requests для HTTP и встроенный json, простая и читаемая.

Go – встроенный net/http и encoding/json, высокая производительность.

Rust – reqwest и serde_json для безопасности и скорости.

C++ – libcurl и nlohmann/json, классика.

Java – HttpClient (Java 11+) и Jackson/Gson (в примере используем javax.json для простоты).

C# – HttpClient и Newtonsoft.Json.

Ruby – net/http и json из стандартной библиотеки.

Node.js – axios и встроенный JSON.

🎮 Использование
text
# Вывод погоды для указанных городов (аргументы)
weather_compare Moscow London Paris

# Интерактивный режим (без аргументов)
weather_compare

# Загрузка списка городов из файла
weather_compare --file cities.txt
В интерактивном режиме доступны команды:

add <город> – добавить город в список

remove <город> – удалить город

show – показать погоду для всех городов

save – сохранить список в favorites.txt

load – загрузить список из favorites.txt

quit – выход

🤝 Вклад
PR и issues приветствуются. Добавляйте поддержку новых API, улучшайте вывод, добавляйте графики.

📄 Лицензия
MIT License.
