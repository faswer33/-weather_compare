# weather_compare.py
import sys
import json
import requests
from datetime import datetime, timedelta
from collections import OrderedDict

CACHE = {}
CACHE_TTL = 300  # 5 минут

def get_weather(city):
    """Получить данные о погоде для города с кешированием."""
    now = datetime.now()
    if city in CACHE and (now - CACHE[city]['time']).total_seconds() < CACHE_TTL:
        return CACHE[city]['data']
    try:
        url = f"https://wttr.in/{city}?format=j1&lang=ru"
        resp = requests.get(url, timeout=10)
        if resp.status_code != 200:
            return None
        data = resp.json()
        current = data.get('current_condition', [{}])[0]
        weather = {
            'city': city,
            'temp': current.get('temp_C', 'N/A'),
            'humidity': current.get('humidity', 'N/A'),
            'wind': current.get('windSpeed', 'N/A'),
            'pressure': current.get('pressure', 'N/A'),
            'cloud': current.get('cloudcover', 'N/A'),
            'desc': current.get('weatherDesc', [{}])[0].get('value', ''),
        }
        CACHE[city] = {'time': now, 'data': weather}
        return weather
    except Exception as e:
        print(f"Ошибка получения погоды для {city}: {e}", file=sys.stderr)
        return None

def print_table(weather_list):
    """Вывод таблицы с погодой."""
    if not weather_list:
        print("Нет данных для отображения.")
        return
    headers = ['Город', 'Темп.', 'Влаж.', 'Ветер', 'Давл.', 'Облач.', 'Описание']
    col_widths = [15, 8, 8, 8, 8, 8, 20]
    # Печать заголовка
    header_line = '|'.join(h.center(w) for h, w in zip(headers, col_widths))
    print('+' + '+'.join('-'*w for w in col_widths) + '+')
    print('|' + header_line + '|')
    print('+' + '+'.join('-'*w for w in col_widths) + '+')
    for w in weather_list:
        if w is None:
            continue
        row = [
            w['city'][:col_widths[0]].center(col_widths[0]),
            f"{w['temp']}°C".center(col_widths[1]),
            f"{w['humidity']}%".center(col_widths[2]),
            f"{w['wind']} км/ч".center(col_widths[3]),
            f"{w['pressure']} мм".center(col_widths[4]),
            f"{w['cloud']}%".center(col_widths[5]),
            w['desc'][:col_widths[6]].center(col_widths[6])
        ]
        print('|' + '|'.join(row) + '|')
    print('+' + '+'.join('-'*w for w in col_widths) + '+')

def interactive():
    """Интерактивный режим."""
    cities = []
    print("Weather Compare (интерактивный режим)")
    print("Команды: add <город>, remove <город>, show, save, load, quit")
    while True:
        cmd = input("> ").strip().split()
        if not cmd:
            continue
        if cmd[0] == 'quit':
            break
        elif cmd[0] == 'add' and len(cmd) > 1:
            city = ' '.join(cmd[1:])
            if city not in cities:
                cities.append(city)
                print(f"Город {city} добавлен.")
            else:
                print(f"Город {city} уже есть.")
        elif cmd[0] == 'remove' and len(cmd) > 1:
            city = ' '.join(cmd[1:])
            if city in cities:
                cities.remove(city)
                print(f"Город {city} удалён.")
            else:
                print(f"Город {city} не найден.")
        elif cmd[0] == 'show':
            if not cities:
                print("Список городов пуст.")
                continue
            weather_list = []
            for city in cities:
                data = get_weather(city)
                if data:
                    weather_list.append(data)
            print_table(weather_list)
        elif cmd[0] == 'save':
            with open('favorites.txt', 'w', encoding='utf-8') as f:
                f.write('\n'.join(cities))
            print("Список сохранён в favorites.txt")
        elif cmd[0] == 'load':
            try:
                with open('favorites.txt', 'r', encoding='utf-8') as f:
                    loaded = [line.strip() for line in f if line.strip()]
                cities = loaded
                print("Список загружен из favorites.txt")
            except FileNotFoundError:
                print("Файл favorites.txt не найден.")
        else:
            print("Неизвестная команда.")

def main():
    if len(sys.argv) == 1:
        interactive()
        return
    cities = sys.argv[1:]
    weather_list = []
    for city in cities:
        data = get_weather(city)
        if data:
            weather_list.append(data)
    print_table(weather_list)

if __name__ == '__main__':
    main()
