// WeatherCompare.java
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.net.URI;
import java.time.Duration;
import java.io.*;
import java.nio.file.*;
import java.util.*;
import javax.json.*;

public class WeatherCompare {
    static class WeatherData {
        String city;
        String temp;
        String humidity;
        String wind;
        String pressure;
        String cloud;
        String desc;
    }

    private static final HttpClient client = HttpClient.newHttpClient();
    private static final Map<String, CacheEntry> cache = new HashMap<>();
    private static final long CACHE_TTL = 300; // seconds

    static class CacheEntry {
        WeatherData data;
        long time;
        CacheEntry(WeatherData data) { this.data=data; this.time=System.currentTimeMillis(); }
    }

    private static WeatherData getWeather(String city) throws Exception {
        // Check cache
        CacheEntry entry = cache.get(city);
        if (entry != null && (System.currentTimeMillis() - entry.time) < CACHE_TTL*1000) {
            return entry.data;
        }
        String url = "https://wttr.in/" + city + "?format=j1&lang=ru";
        HttpRequest request = HttpRequest.newBuilder()
                .uri(URI.create(url))
                .timeout(Duration.ofSeconds(10))
                .build();
        HttpResponse<String> response = client.send(request, HttpResponse.BodyHandlers.ofString());
        if (response.statusCode() != 200) {
            throw new Exception("HTTP error: " + response.statusCode());
        }
        String json = response.body();
        JsonReader reader = Json.createReader(new StringReader(json));
        JsonObject obj = reader.readObject();
        reader.close();
        JsonArray current = obj.getJsonArray("current_condition");
        if (current.isEmpty()) {
            throw new Exception("No current_condition");
        }
        JsonObject cond = current.getJsonObject(0);
        WeatherData w = new WeatherData();
        w.city = city;
        w.temp = cond.getString("temp_C", "N/A");
        w.humidity = cond.getString("humidity", "N/A");
        w.wind = cond.getString("windSpeed", "N/A");
        w.pressure = cond.getString("pressure", "N/A");
        w.cloud = cond.getString("cloudcover", "N/A");
        JsonArray descArr = cond.getJsonArray("weatherDesc");
        w.desc = descArr.isEmpty() ? "" : descArr.getJsonObject(0).getString("value", "");
        cache.put(city, new CacheEntry(w));
        return w;
    }

    private static String center(String s, int width) {
        if (s.length() >= width) return s.substring(0, width);
        int left = (width - s.length()) / 2;
        int right = width - s.length() - left;
        return " ".repeat(left) + s + " ".repeat(right);
    }

    private static void printSeparator(int[] widths) {
        System.out.print("+");
        for (int w : widths) System.out.print("-".repeat(w) + "+");
        System.out.println();
    }

    private static void printRow(List<String> row, int[] widths) {
        System.out.print("|");
        for (int i=0; i<row.size(); i++) {
            System.out.print(center(row.get(i), widths[i]) + "|");
        }
        System.out.println();
    }

    private static void printTable(List<WeatherData> list) {
        if (list.isEmpty()) {
            System.out.println("Нет данных для отображения.");
            return;
        }
        String[] headers = {"Город", "Темп.", "Влаж.", "Ветер", "Давл.", "Облач.", "Описание"};
        int[] widths = {15,8,8,8,8,8,20};
        printSeparator(widths);
        printRow(Arrays.asList(headers), widths);
        printSeparator(widths);
        for (WeatherData w : list) {
            List<String> row = Arrays.asList(
                center(w.city, widths[0]),
                center(w.temp + "°C", widths[1]),
                center(w.humidity + "%", widths[2]),
                center(w.wind + " км/ч", widths[3]),
                center(w.pressure + " мм", widths[4]),
                center(w.cloud + "%", widths[5]),
                center(w.desc, widths[6])
            );
            printRow(row, widths);
        }
        printSeparator(widths);
    }

    private static void interactive() throws Exception {
        List<String> cities = new ArrayList<>();
        BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
        System.out.println("Weather Compare (интерактивный режим)");
        System.out.println("Команды: add <город>, remove <город>, show, save, load, quit");
        while (true) {
            System.out.print("> ");
            String line = br.readLine();
            if (line == null) break;
            String[] parts = line.trim().split(" ");
            if (parts.length == 0) continue;
            switch (parts[0]) {
                case "quit":
                    return;
                case "add":
                    if (parts.length < 2) { System.out.println("Укажите город."); continue; }
                    String city = line.substring(parts[0].length()+1);
                    if (!cities.contains(city)) {
                        cities.add(city);
                        System.out.println("Город " + city + " добавлен.");
                    } else {
                        System.out.println("Город " + city + " уже есть.");
                    }
                    break;
                case "remove":
                    if (parts.length < 2) { System.out.println("Укажите город."); continue; }
                    city = line.substring(parts[0].length()+1);
                    if (cities.remove(city)) {
                        System.out.println("Город " + city + " удалён.");
                    } else {
                        System.out.println("Город " + city + " не найден.");
                    }
                    break;
                case "show":
                    if (cities.isEmpty()) {
                        System.out.println("Список городов пуст.");
                        continue;
                    }
                    List<WeatherData> weatherList = new ArrayList<>();
                    for (String c : cities) {
                        try {
                            weatherList.add(getWeather(c));
                        } catch (Exception e) {
                            System.out.println("Ошибка для " + c + ": " + e.getMessage());
                        }
                    }
                    printTable(weatherList);
                    break;
                case "save":
                    Files.write(Paths.get("favorites.txt"), cities);
                    System.out.println("Список сохранён в favorites.txt");
                    break;
                case "load":
                    try {
                        List<String> loaded = Files.readAllLines(Paths.get("favorites.txt"));
                        cities.clear();
                        cities.addAll(loaded);
                        System.out.println("Список загружен из favorites.txt");
                    } catch (IOException e) {
                        System.out.println("Файл favorites.txt не найден.");
                    }
                    break;
                default:
                    System.out.println("Неизвестная команда.");
            }
        }
    }

    public static void main(String[] args) throws Exception {
        if (args.length == 0) {
            interactive();
        } else {
            List<WeatherData> weatherList = new ArrayList<>();
            for (String city : args) {
                try {
                    weatherList.add(getWeather(city));
                } catch (Exception e) {
                    System.out.println("Ошибка для " + city + ": " + e.getMessage());
                }
            }
            printTable(weatherList);
        }
    }
}
