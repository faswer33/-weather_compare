// weather_compare.rs
use reqwest;
use serde_json::Value;
use std::collections::HashMap;
use std::env;
use std::io::{self, Write};
use std::time::{Duration, Instant};

struct WeatherData {
    city: String,
    temp: String,
    humidity: String,
    wind: String,
    pressure: String,
    cloud: String,
    desc: String,
}

struct CacheEntry {
    data: WeatherData,
    time: Instant,
}

lazy_static::lazy_static! {
    static ref CACHE: std::sync::Mutex<HashMap<String, CacheEntry>> = std::sync::Mutex::new(HashMap::new());
}

const CACHE_TTL: Duration = Duration::from_secs(300);

fn get_weather(city: &str) -> Result<WeatherData, String> {
    // Check cache
    {
        let cache = CACHE.lock().unwrap();
        if let Some(entry) = cache.get(city) {
            if entry.time.elapsed() < CACHE_TTL {
                return Ok(entry.data.clone());
            }
        }
    }
    let url = format!("https://wttr.in/{}?format=j1&lang=ru", city);
    let response = reqwest::blocking::get(&url).map_err(|e| e.to_string())?;
    if !response.status().is_success() {
        return Err(format!("HTTP error: {}", response.status()));
    }
    let text = response.text().map_err(|e| e.to_string())?;
    let json: Value = serde_json::from_str(&text).map_err(|e| e.to_string())?;
    let current = json["current_condition"][0].as_object().ok_or("No current_condition")?;
    let temp = current["temp_C"].as_str().unwrap_or("N/A").to_string();
    let humidity = current["humidity"].as_str().unwrap_or("N/A").to_string();
    let wind = current["windSpeed"].as_str().unwrap_or("N/A").to_string();
    let pressure = current["pressure"].as_str().unwrap_or("N/A").to_string();
    let cloud = current["cloudcover"].as_str().unwrap_or("N/A").to_string();
    let desc = current["weatherDesc"][0]["value"].as_str().unwrap_or("").to_string();
    let weather = WeatherData {
        city: city.to_string(),
        temp,
        humidity,
        wind,
        pressure,
        cloud,
        desc,
    };
    // Update cache
    {
        let mut cache = CACHE.lock().unwrap();
        cache.insert(city.to_string(), CacheEntry { data: weather.clone(), time: Instant::now() });
    }
    Ok(weather)
}

fn print_table(weather_list: &[WeatherData]) {
    if weather_list.is_empty() {
        println!("Нет данных для отображения.");
        return;
    }
    let headers = vec!["Город", "Темп.", "Влаж.", "Ветер", "Давл.", "Облач.", "Описание"];
    let col_widths = vec![15, 8, 8, 8, 8, 8, 20];
    print_separator(&col_widths);
    print_row(&headers, &col_widths);
    print_separator(&col_widths);
    for w in weather_list {
        let row = vec![
            center(&w.city, col_widths[0]),
            center(&format!("{}°C", w.temp), col_widths[1]),
            center(&format!("{}%", w.humidity), col_widths[2]),
            center(&format!("{} км/ч", w.wind), col_widths[3]),
            center(&format!("{} мм", w.pressure), col_widths[4]),
            center(&format!("{}%", w.cloud), col_widths[5]),
            center(&w.desc, col_widths[6]),
        ];
        print_row(&row, &col_widths);
    }
    print_separator(&col_widths);
}

fn print_separator(widths: &[usize]) {
    print!("+");
    for w in widths {
        print!("{}", "-".repeat(*w));
        print!("+");
    }
    println!();
}

fn print_row(row: &[String], widths: &[usize]) {
    print!("|");
    for (i, s) in row.iter().enumerate() {
        print!("{}", center(s, widths[i]));
        print!("|");
    }
    println!();
}

fn center(s: &str, width: usize) -> String {
    if s.len() >= width {
        return s[..width].to_string();
    }
    let left = (width - s.len()) / 2;
    let right = width - s.len() - left;
    format!("{}{}{}", " ".repeat(left), s, " ".repeat(right))
}

fn interactive() {
    let mut cities: Vec<String> = Vec::new();
    println!("Weather Compare (интерактивный режим)");
    println!("Команды: add <город>, remove <город>, show, save, load, quit");
    loop {
        print!("> ");
        io::stdout().flush().unwrap();
        let mut input = String::new();
        io::stdin().read_line(&mut input).unwrap();
        let parts: Vec<&str> = input.trim().split_whitespace().collect();
        if parts.is_empty() {
            continue;
        }
        match parts[0] {
            "quit" => break,
            "add" => {
                if parts.len() < 2 {
                    println!("Укажите город.");
                    continue;
                }
                let city = parts[1..].join(" ");
                if !cities.contains(&city) {
                    cities.push(city.clone());
                    println!("Город {} добавлен.", city);
                } else {
                    println!("Город {} уже есть.", city);
                }
            }
            "remove" => {
                if parts.len() < 2 {
                    println!("Укажите город.");
                    continue;
                }
                let city = parts[1..].join(" ");
                if let Some(pos) = cities.iter().position(|c| c == &city) {
                    cities.remove(pos);
                    println!("Город {} удалён.", city);
                } else {
                    println!("Город {} не найден.", city);
                }
            }
            "show" => {
                if cities.is_empty() {
                    println!("Список городов пуст.");
                    continue;
                }
                let mut weather_list = Vec::new();
                for city in &cities {
                    match get_weather(city) {
                        Ok(w) => weather_list.push(w),
                        Err(e) => println!("Ошибка для {}: {}", city, e),
                    }
                }
                print_table(&weather_list);
            }
            "save" => {
                if let Ok(mut f) = std::fs::File::create("favorites.txt") {
                    for city in &cities {
                        writeln!(f, "{}", city).unwrap();
                    }
                    println!("Список сохранён в favorites.txt");
                } else {
                    println!("Ошибка сохранения.");
                }
            }
            "load" => {
                if let Ok(content) = std::fs::read_to_string("favorites.txt") {
                    cities = content.lines().map(|s| s.trim().to_string()).filter(|s| !s.is_empty()).collect();
                    println!("Список загружен из favorites.txt");
                } else {
                    println!("Файл favorites.txt не найден.");
                }
            }
            _ => println!("Неизвестная команда."),
        }
    }
}

fn main() {
    let args: Vec<String> = env::args().collect();
    if args.len() == 1 {
        interactive();
        return;
    }
    let cities = &args[1..];
    let mut weather_list = Vec::new();
    for city in cities {
        match get_weather(city) {
            Ok(w) => weather_list.push(w),
            Err(e) => println!("Ошибка для {}: {}", city, e),
        }
    }
    print_table(&weather_list);
}
