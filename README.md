# Проект: LED Control с использованием Raspberry Pi и ESP32

## 📋 Описание

Данный проект позволяет управлять светодиодами (LED1, LED2, LED3), подключёнными к ESP32, через веб-интерфейс или API на Raspberry Pi.

---

## 🧠 Компоненты

### ESP32:
- Поднимает веб-сервер на порту `80`.
- Доступен по mDNS-имени: `esp32block.local`.
- Обрабатывает пути:  
  `/led1_on`, `/led1_off`, `/led1_blink`  
  `/led2_on`, `/led2_off`, `/led2_blink`  
  `/led3_on`, `/led3_off`, `/led3_blink`

### Raspberry Pi:
- Запускает Flask-приложение на порту `5000`.
- Предоставляет:
  - API (`/api/ledX/action`)
  - Веб-интерфейс (Bootstrap)
  - Панель статуса соединения с ESP32

Работает локально или через интернет (с пробросом портов, VPN и т.д.).

---

## 🗂️ Структура проекта

```
/home/pi/led_control/
├── app.py               # Flask-приложение
├── templates/
│   └── index.html       # HTML-шаблон управления
├── static/              # (опционально, стили/скрипты)
└── README.md            # текущий файл описания
```

---

## ⚙️ Установка

### Установка Python и pip:
```bash
sudo apt-get update
sudo apt-get install -y python3 python3-pip
```

### Установка зависимостей:
```bash
pip3 install flask requests
```

### Клонирование проекта:
Скопируйте проект в `/home/pi/led_control/`.

---

## 🔧 Настройка ESP32

- Залить скетч, который поднимает HTTP-сервер на `esp32block.local`.
- Убедиться, что mDNS работает в сети (выполнить `ping esp32block.local`).

---

## 🚀 Запуск вручную

```bash
cd /home/pi/led_control
python3 app.py
```

Открыть браузер:  
`http://<IP_малинки>:5000`  
(например: `http://192.168.1.10:5000`)

---

## 🔁 Автозапуск через systemd

Создание сервиса:

```bash
sudo nano /etc/systemd/system/ledcontrol.service
```

Пример содержимого:
```ini
[Unit]
Description=LED Control Flask Service
After=network-online.target
Wants=network-online.target

[Service]
User=pi
WorkingDirectory=/home/pi/led_control
ExecStart=/usr/bin/python3 /home/pi/led_control/app.py
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
```

Применение:
```bash
sudo systemctl daemon-reload
sudo systemctl enable ledcontrol.service
sudo systemctl start ledcontrol.service
```

Проверка:
```bash
systemctl status ledcontrol.service
```

---

## 🧪 Тестирование

Проверить доступность ESP32:
```bash
ping esp32block.local
```

Открыть интерфейс:
```
http://192.168.x.x:5000
```

Нажать кнопку "LED1 ON" — соответствующий светодиод должен включиться.  
Если ESP32 недоступен, статус покажет "ESP32 недоступен".

---

## 🌐 API

Примеры вызовов:
```
GET /api/led1/on
GET /api/led1/off
GET /api/led1/blink
```

Пример:
```bash
curl http://<IP_малинки>:5000/api/led1/on
```

Ответ:
```json
{
  "status": "success",
  "message": "LED1 is ON permanently"
}
```

---

## 💡 Дополнительно

- Убедитесь, что службы Bonjour/mDNS активны (особенно на Windows).
- При необходимости можно заменить `esp32block.local` на IP в `ESP32_BASE_URL` в `app.py`.

---

## 🛠 Авторы

- 🧠 Backend: Raspberry Pi + Flask
- ⚡️ Hardware: ESP32 (Wi-Fi, WebServer)
- 🖥 UI: Bootstrap HTML (в `templates/index.html`)

---

## 🧾 Лицензия

Проект открыт для модификации и использования в образовательных целях.
