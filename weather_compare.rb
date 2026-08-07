# weather_compare.rb
require 'net/http'
require 'json'
require 'time'

class WeatherData
  attr_accessor :city, :temp, :humidity, :wind, :pressure, :cloud, :desc
end

$cache = {}
CACHE_TTL = 300  # seconds

def get_weather(city)
  now = Time.now
  if $cache.key?(city) && (now - $cache[city][:time]).to_i < CACHE_TTL
    return $cache[city][:data]
  end
  url = "https://wttr.in/#{city}?format=j1&lang=ru"
  uri = URI(url)
  response = Net::HTTP.get_response(uri)
  unless response.is_a?(Net::HTTPSuccess)
    raise "HTTP error: #{response.code}"
  end
  data = JSON.parse(response.body)
  current = data['current_condition']&.first
  raise "No current_condition" unless current
  w = WeatherData.new
  w.city = city
  w.temp = current['temp_C'] || 'N/A'
  w.humidity = current['humidity'] || 'N/A'
  w.wind = current['windSpeed'] || 'N/A'
  w.pressure = current['pressure'] || 'N/A'
  w.cloud = current['cloudcover'] || 'N/A'
  w.desc = current['weatherDesc']&.first&.fetch('value', '') || ''
  $cache[city] = {data: w, time: now}
  w
end

def center(s, width)
  if s.length >= width
    s[0, width]
  else
    left = (width - s.length) / 2
    right = width - s.length - left
    ' ' * left + s + ' ' * right
  end
end

def print_separator(widths)
  print '+'
  widths.each { |w| print '-' * w + '+' }
  puts
end

def print_row(row, widths)
  print '|'
  row.each_with_index { |s, i| print center(s, widths[i]) + '|' }
  puts
end

def print_table(weather_list)
  if weather_list.empty?
    puts "Нет данных для отображения."
    return
  end
  headers = ['Город', 'Темп.', 'Влаж.', 'Ветер', 'Давл.', 'Облач.', 'Описание']
  widths = [15,8,8,8,8,8,20]
  print_separator(widths)
  print_row(headers, widths)
  print_separator(widths)
  weather_list.each do |w|
    row = [
      center(w.city, widths[0]),
      center(w.temp + '°C', widths[1]),
      center(w.humidity + '%', widths[2]),
      center(w.wind + ' км/ч', widths[3]),
      center(w.pressure + ' мм', widths[4]),
      center(w.cloud + '%', widths[5]),
      center(w.desc, widths[6])
    ]
    print_row(row, widths)
  end
  print_separator(widths)
end

def interactive
  cities = []
  puts "Weather Compare (интерактивный режим)"
  puts "Команды: add <город>, remove <город>, show, save, load, quit"
  loop do
    print "> "
    line = gets.chomp.strip
    next if line.empty?
    parts = line.split
    case parts[0]
    when 'quit'
      break
    when 'add'
      if parts.size < 2
        puts "Укажите город."
        next
      end
      city = line[parts[0].length+1..-1]
      unless cities.include?(city)
        cities << city
        puts "Город #{city} добавлен."
      else
        puts "Город #{city} уже есть."
      end
    when 'remove'
      if parts.size < 2
        puts "Укажите город."
        next
      end
      city = line[parts[0].length+1..-1]
      if cities.delete(city)
        puts "Город #{city} удалён."
      else
        puts "Город #{city} не найден."
      end
    when 'show'
      if cities.empty?
        puts "Список городов пуст."
        next
      end
      weather_list = []
      cities.each do |c|
        begin
          weather_list << get_weather(c)
        rescue => e
          puts "Ошибка для #{c}: #{e.message}"
        end
      end
      print_table(weather_list)
    when 'save'
      File.write('favorites.txt', cities.join("\n"))
      puts "Список сохранён в favorites.txt"
    when 'load'
      if File.exist?('favorites.txt')
        loaded = File.readlines('favorites.txt').map(&:strip).reject(&:empty?)
        cities = loaded
        puts "Список загружен из favorites.txt"
      else
        puts "Файл favorites.txt не найден."
      end
    else
      puts "Неизвестная команда."
    end
  end
end

if __FILE__ == $0
  if ARGV.empty?
    interactive
  else
    weather_list = []
    ARGV.each do |city|
      begin
        weather_list << get_weather(city)
      rescue => e
        puts "Ошибка для #{city}: #{e.message}"
      end
    end
    print_table(weather_list)
  end
end
