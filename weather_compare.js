// weather_compare.js
const axios = require('axios');
const fs = require('fs');
const readline = require('readline');

const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout
});

const cache = {};
const CACHE_TTL = 300 * 1000; // milliseconds

async function getWeather(city) {
    const now = Date.now();
    if (cache[city] && (now - cache[city].time < CACHE_TTL)) {
        return cache[city].data;
    }
    try {
        const url = `https://wttr.in/${city}?format=j1&lang=ru`;
        const resp = await axios.get(url, { timeout: 10000 });
        if (resp.status !== 200) throw new Error(`HTTP error: ${resp.status}`);
        const data = resp.data;
        const current = data.current_condition && data.current_condition[0];
        if (!current) throw new Error('No current_condition');
        const weather = {
            city: city,
            temp: current.temp_C || 'N/A',
            humidity: current.humidity || 'N/A',
            wind: current.windSpeed || 'N/A',
            pressure: current.pressure || 'N/A',
            cloud: current.cloudcover || 'N/A',
            desc: (current.weatherDesc && current.weatherDesc[0] && current.weatherDesc[0].value) || ''
        };
        cache[city] = { data: weather, time: now };
        return weather;
    } catch (e) {
        throw new Error(`Ошибка получения погоды для ${city}: ${e.message}`);
    }
}

function center(s, width) {
    if (s.length >= width) return s.substring(0, width);
    const left = Math.floor((width - s.length) / 2);
    const right = width - s.length - left;
    return ' '.repeat(left) + s + ' '.repeat(right);
}

function printSeparator(widths) {
    process.stdout.write('+');
    widths.forEach(w => process.stdout.write('-'.repeat(w) + '+'));
    console.log();
}

function printRow(row, widths) {
    process.stdout.write('|');
    row.forEach((s, i) => process.stdout.write(center(s, widths[i]) + '|'));
    console.log();
}

function printTable(weatherList) {
    if (weatherList.length === 0) {
        console.log('Нет данных для отображения.');
        return;
    }
    const headers = ['Город', 'Темп.', 'Влаж.', 'Ветер', 'Давл.', 'Облач.', 'Описание'];
    const widths = [15, 8, 8, 8, 8, 8, 20];
    printSeparator(widths);
    printRow(headers, widths);
    printSeparator(widths);
    weatherList.forEach(w => {
        const row = [
            center(w.city, widths[0]),
            center(w.temp + '°C', widths[1]),
            center(w.humidity + '%', widths[2]),
            center(w.wind + ' км/ч', widths[3]),
            center(w.pressure + ' мм', widths[4]),
            center(w.cloud + '%', widths[5]),
            center(w.desc, widths[6])
        ];
        printRow(row, widths);
    });
    printSeparator(widths);
}

function interactive() {
    const cities = [];
    console.log('Weather Compare (интерактивный режим)');
    console.log('Команды: add <город>, remove <город>, show, save, load, quit');
    const prompt = () => {
        rl.question('> ', async (line) => {
            const parts = line.trim().split(/\s+/);
            if (parts.length === 0) { prompt(); return; }
            switch (parts[0]) {
                case 'quit':
                    rl.close();
                    return;
                case 'add':
                    if (parts.length < 2) {
                        console.log('Укажите город.');
                        prompt(); return;
                    }
                    const city = line.substring(parts[0].length + 1);
                    if (!cities.includes(city)) {
                        cities.push(city);
                        console.log(`Город ${city} добавлен.`);
                    } else {
                        console.log(`Город ${city} уже есть.`);
                    }
                    prompt();
                    break;
                case 'remove':
                    if (parts.length < 2) {
                        console.log('Укажите город.');
                        prompt(); return;
                    }
                    const cityRem = line.substring(parts[0].length + 1);
                    const idx = cities.indexOf(cityRem);
                    if (idx !== -1) {
                        cities.splice(idx, 1);
                        console.log(`Город ${cityRem} удалён.`);
                    } else {
                        console.log(`Город ${cityRem} не найден.`);
                    }
                    prompt();
                    break;
                case 'show':
                    if (cities.length === 0) {
                        console.log('Список городов пуст.');
                        prompt(); return;
                    }
                    const weatherList = [];
                    for (const c of cities) {
                        try {
                            weatherList.push(await getWeather(c));
                        } catch (e) {
                            console.log(e.message);
                        }
                    }
                    printTable(weatherList);
                    prompt();
                    break;
                case 'save':
                    fs.writeFileSync('favorites.txt', cities.join('\n'));
                    console.log('Список сохранён в favorites.txt');
                    prompt();
                    break;
                case 'load':
                    try {
                        const data = fs.readFileSync('favorites.txt', 'utf-8');
                        const loaded = data.split('\n').map(s => s.trim()).filter(s => s);
                        cities.length = 0;
                        cities.push(...loaded);
                        console.log('Список загружен из favorites.txt');
                    } catch (e) {
                        console.log('Файл favorites.txt не найден.');
                    }
                    prompt();
                    break;
                default:
                    console.log('Неизвестная команда.');
                    prompt();
            }
        });
    };
    prompt();
}

async function main() {
    const args = process.argv.slice(2);
    if (args.length === 0) {
        interactive();
    } else {
        const weatherList = [];
        for (const city of args) {
            try {
                weatherList.push(await getWeather(city));
            } catch (e) {
                console.log(e.message);
            }
        }
        printTable(weatherList);
    }
}

main().catch(console.error);
