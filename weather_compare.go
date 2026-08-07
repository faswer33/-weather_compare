// weather_compare.go
package main

import (
	"bufio"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"os"
	"strings"
	"time"
)

type WeatherData struct {
	City     string
	Temp     string
	Humidity string
	Wind     string
	Pressure string
	Cloud    string
	Desc     string
}

type WttrResponse struct {
	CurrentCondition []struct {
		TempC      string `json:"temp_C"`
		Humidity   string `json:"humidity"`
		WindSpeed  string `json:"windSpeed"`
		Pressure   string `json:"pressure"`
		Cloudcover string `json:"cloudcover"`
		WeatherDesc []struct {
			Value string `json:"value"`
		} `json:"weatherDesc"`
	} `json:"current_condition"`
}

var cache = make(map[string]struct {
	data WeatherData
	time time.Time
})
const cacheTTL = 300

func getWeather(city string) (*WeatherData, error) {
	if entry, ok := cache[city]; ok && time.Since(entry.time).Seconds() < cacheTTL {
		return &entry.data, nil
	}
	url := fmt.Sprintf("https://wttr.in/%s?format=j1&lang=ru", city)
	resp, err := http.Get(url)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()
	body, err := io.ReadAll(resp.Body)
	if err != nil {
		return nil, err
	}
	var wttr WttrResponse
	err = json.Unmarshal(body, &wttr)
	if err != nil {
		return nil, err
	}
	if len(wttr.CurrentCondition) == 0 {
		return nil, fmt.Errorf("no data for %s", city)
	}
	cond := wttr.CurrentCondition[0]
	desc := ""
	if len(cond.WeatherDesc) > 0 {
		desc = cond.WeatherDesc[0].Value
	}
	weather := WeatherData{
		City:     city,
		Temp:     cond.TempC,
		Humidity: cond.Humidity,
		Wind:     cond.WindSpeed,
		Pressure: cond.Pressure,
		Cloud:    cond.Cloudcover,
		Desc:     desc,
	}
	cache[city] = struct {
		data WeatherData
		time time.Time
	}{weather, time.Now()}
	return &weather, nil
}

func printTable(weatherList []WeatherData) {
	if len(weatherList) == 0 {
		fmt.Println("Нет данных для отображения.")
		return
	}
	headers := []string{"Город", "Темп.", "Влаж.", "Ветер", "Давл.", "Облач.", "Описание"}
	colWidths := []int{15, 8, 8, 8, 8, 8, 20}
	// заголовок
	printSeparator(colWidths)
	printRow(headers, colWidths)
	printSeparator(colWidths)
	for _, w := range weatherList {
		row := []string{
			center(w.City, colWidths[0]),
			center(w.Temp+"°C", colWidths[1]),
			center(w.Humidity+"%", colWidths[2]),
			center(w.Wind+" км/ч", colWidths[3]),
			center(w.Pressure+" мм", colWidths[4]),
			center(w.Cloud+"%", colWidths[5]),
			center(w.Desc, colWidths[6]),
		}
		printRow(row, colWidths)
	}
	printSeparator(colWidths)
}

func printSeparator(widths []int) {
	fmt.Print("+")
	for _, w := range widths {
		fmt.Print(strings.Repeat("-", w) + "+")
	}
	fmt.Println()
}

func printRow(row []string, widths []int) {
	fmt.Print("|")
	for i, s := range row {
		fmt.Print(center(s, widths[i]) + "|")
	}
	fmt.Println()
}

func center(s string, width int) string {
	if len(s) >= width {
		return s[:width]
	}
	left := (width - len(s)) / 2
	right := width - len(s) - left
	return strings.Repeat(" ", left) + s + strings.Repeat(" ", right)
}

func interactive() {
	scanner := bufio.NewScanner(os.Stdin)
	cities := []string{}
	fmt.Println("Weather Compare (интерактивный режим)")
	fmt.Println("Команды: add <город>, remove <город>, show, save, load, quit")
	for {
		fmt.Print("> ")
		if !scanner.Scan() {
			break
		}
		parts := strings.Fields(scanner.Text())
		if len(parts) == 0 {
			continue
		}
		switch parts[0] {
		case "quit":
			return
		case "add":
			if len(parts) < 2 {
				fmt.Println("Укажите город.")
				continue
			}
			city := strings.Join(parts[1:], " ")
			if !contains(cities, city) {
				cities = append(cities, city)
				fmt.Printf("Город %s добавлен.\n", city)
			} else {
				fmt.Printf("Город %s уже есть.\n", city)
			}
		case "remove":
			if len(parts) < 2 {
				fmt.Println("Укажите город.")
				continue
			}
			city := strings.Join(parts[1:], " ")
			for i, c := range cities {
				if c == city {
					cities = append(cities[:i], cities[i+1:]...)
					fmt.Printf("Город %s удалён.\n", city)
					break
				}
			}
		case "show":
			if len(cities) == 0 {
				fmt.Println("Список городов пуст.")
				continue
			}
			var weatherList []WeatherData
			for _, city := range cities {
				w, err := getWeather(city)
				if err != nil {
					fmt.Printf("Ошибка для %s: %v\n", city, err)
					continue
				}
				weatherList = append(weatherList, *w)
			}
			printTable(weatherList)
		case "save":
			f, err := os.Create("favorites.txt")
			if err != nil {
				fmt.Println("Ошибка сохранения:", err)
				continue
			}
			for _, c := range cities {
				fmt.Fprintln(f, c)
			}
			f.Close()
			fmt.Println("Список сохранён в favorites.txt")
		case "load":
			f, err := os.Open("favorites.txt")
			if err != nil {
				fmt.Println("Файл favorites.txt не найден.")
				continue
			}
			defer f.Close()
			sc := bufio.NewScanner(f)
			cities = []string{}
			for sc.Scan() {
				line := strings.TrimSpace(sc.Text())
				if line != "" {
					cities = append(cities, line)
				}
			}
			fmt.Println("Список загружен из favorites.txt")
		default:
			fmt.Println("Неизвестная команда.")
		}
	}
}

func contains(slice []string, item string) bool {
	for _, s := range slice {
		if s == item {
			return true
		}
	}
	return false
}

func main() {
	if len(os.Args) == 1 {
		interactive()
		return
	}
	cities := os.Args[1:]
	var weatherList []WeatherData
	for _, city := range cities {
		w, err := getWeather(city)
		if err != nil {
			fmt.Printf("Ошибка для %s: %v\n", city, err)
			continue
		}
		weatherList = append(weatherList, *w)
	}
	printTable(weatherList)
}
