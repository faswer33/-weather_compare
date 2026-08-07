// weather_compare.cs
using System;
using System.Collections.Generic;
using System.IO;
using System.Net.Http;
using System.Threading.Tasks;
using System.Linq;
using Newtonsoft.Json.Linq;

class WeatherData
{
    public string City { get; set; }
    public string Temp { get; set; }
    public string Humidity { get; set; }
    public string Wind { get; set; }
    public string Pressure { get; set; }
    public string Cloud { get; set; }
    public string Desc { get; set; }
}

class Program
{
    private static readonly HttpClient client = new HttpClient();
    private static Dictionary<string, (WeatherData data, DateTime time)> cache = new Dictionary<string, (WeatherData, DateTime)>();
    private static readonly TimeSpan cacheTTL = TimeSpan.FromSeconds(300);

    static async Task<WeatherData> GetWeather(string city)
    {
        // Check cache
        if (cache.TryGetValue(city, out var entry) && DateTime.Now - entry.time < cacheTTL)
            return entry.data;

        string url = $"https://wttr.in/{city}?format=j1&lang=ru";
        var response = await client.GetAsync(url);
        if (!response.IsSuccessStatusCode)
            throw new Exception($"HTTP error: {response.StatusCode}");
        string json = await response.Content.ReadAsStringAsync();
        var obj = JObject.Parse(json);
        var current = obj["current_condition"]?[0];
        if (current == null)
            throw new Exception("No current_condition");
        var w = new WeatherData
        {
            City = city,
            Temp = (string)current["temp_C"] ?? "N/A",
            Humidity = (string)current["humidity"] ?? "N/A",
            Wind = (string)current["windSpeed"] ?? "N/A",
            Pressure = (string)current["pressure"] ?? "N/A",
            Cloud = (string)current["cloudcover"] ?? "N/A",
            Desc = (string)current["weatherDesc"]?[0]?["value"] ?? ""
        };
        cache[city] = (w, DateTime.Now);
        return w;
    }

    static string Center(string s, int width)
    {
        if (s.Length >= width) return s.Substring(0, width);
        int left = (width - s.Length) / 2;
        int right = width - s.Length - left;
        return new string(' ', left) + s + new string(' ', right);
    }

    static void PrintSeparator(int[] widths)
    {
        Console.Write("+");
        foreach (int w in widths) Console.Write(new string('-', w) + "+");
        Console.WriteLine();
    }

    static void PrintRow(string[] row, int[] widths)
    {
        Console.Write("|");
        for (int i=0; i<row.Length; i++)
            Console.Write(Center(row[i], widths[i]) + "|");
        Console.WriteLine();
    }

    static void PrintTable(List<WeatherData> list)
    {
        if (list.Count == 0)
        {
            Console.WriteLine("Нет данных для отображения.");
            return;
        }
        string[] headers = { "Город", "Темп.", "Влаж.", "Ветер", "Давл.", "Облач.", "Описание" };
        int[] widths = { 15, 8, 8, 8, 8, 8, 20 };
        PrintSeparator(widths);
        PrintRow(headers, widths);
        PrintSeparator(widths);
        foreach (var w in list)
        {
            string[] row = {
                Center(w.City, widths[0]),
                Center(w.Temp + "°C", widths[1]),
                Center(w.Humidity + "%", widths[2]),
                Center(w.Wind + " км/ч", widths[3]),
                Center(w.Pressure + " мм", widths[4]),
                Center(w.Cloud + "%", widths[5]),
                Center(w.Desc, widths[6])
            };
            PrintRow(row, widths);
        }
        PrintSeparator(widths);
    }

    static async Task Interactive()
    {
        var cities = new List<string>();
        Console.WriteLine("Weather Compare (интерактивный режим)");
        Console.WriteLine("Команды: add <город>, remove <город>, show, save, load, quit");
        while (true)
        {
            Console.Write("> ");
            string line = Console.ReadLine();
            if (line == null) break;
            var parts = line.Split(' ', StringSplitOptions.RemoveEmptyEntries);
            if (parts.Length == 0) continue;
            switch (parts[0])
            {
                case "quit":
                    return;
                case "add":
                    if (parts.Length < 2) { Console.WriteLine("Укажите город."); continue; }
                    string city = line.Substring(parts[0].Length + 1);
                    if (!cities.Contains(city))
                    {
                        cities.Add(city);
                        Console.WriteLine($"Город {city} добавлен.");
                    }
                    else
                        Console.WriteLine($"Город {city} уже есть.");
                    break;
                case "remove":
                    if (parts.Length < 2) { Console.WriteLine("Укажите город."); continue; }
                    city = line.Substring(parts[0].Length + 1);
                    if (cities.Remove(city))
                        Console.WriteLine($"Город {city} удалён.");
                    else
                        Console.WriteLine($"Город {city} не найден.");
                    break;
                case "show":
                    if (cities.Count == 0)
                    {
                        Console.WriteLine("Список городов пуст.");
                        continue;
                    }
                    var weatherList = new List<WeatherData>();
                    foreach (var c in cities)
                    {
                        try
                        {
                            weatherList.Add(await GetWeather(c));
                        }
                        catch (Exception e)
                        {
                            Console.WriteLine($"Ошибка для {c}: {e.Message}");
                        }
                    }
                    PrintTable(weatherList);
                    break;
                case "save":
                    File.WriteAllLines("favorites.txt", cities);
                    Console.WriteLine("Список сохранён в favorites.txt");
                    break;
                case "load":
                    if (File.Exists("favorites.txt"))
                    {
                        var loaded = File.ReadAllLines("favorites.txt").Where(s => !string.IsNullOrWhiteSpace(s)).ToList();
                        cities = loaded;
                        Console.WriteLine("Список загружен из favorites.txt");
                    }
                    else
                        Console.WriteLine("Файл favorites.txt не найден.");
                    break;
                default:
                    Console.WriteLine("Неизвестная команда.");
                    break;
            }
        }
    }

    static async Task Main(string[] args)
    {
        if (args.Length == 0)
        {
            await Interactive();
        }
        else
        {
            var weatherList = new List<WeatherData>();
            foreach (var city in args)
            {
                try
                {
                    weatherList.Add(await GetWeather(city));
                }
                catch (Exception e)
                {
                    Console.WriteLine($"Ошибка для {city}: {e.Message}");
                }
            }
            PrintTable(weatherList);
        }
    }
}
