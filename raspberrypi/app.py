from flask import Flask, render_template, request, jsonify
import requests

app = Flask(__name__)

ESP32_BASE_URL = "http://esp32block.local"

@app.route('/api/<led_id>/<action>', methods=['GET'])
def control_led(led_id, action):
    esp_url = f"{ESP32_BASE_URL}/{led_id}_{action}"
    try:
        resp = requests.get(esp_url, timeout=2)
        if resp.status_code == 200:
            return jsonify({"status": "success", "message": resp.text}), 200
        else:
            return jsonify({"status": "error", "message": f"ESP32 returned {resp.status_code}"}), 500
    except requests.exceptions.RequestException as e:
        return jsonify({"status": "error", "message": str(e)}), 500

# Новый эндпоинт для проверки доступности ESP32
@app.route('/api/ping', methods=['GET'])
def ping_esp32():
    try:
        # Просто сделаем GET к корневой странице ESP32, или к любому доступному эндпоинту
        test_url = ESP32_BASE_URL + "/"
        resp = requests.get(test_url, timeout=2)
        if resp.status_code == 200:
            return jsonify({"status": "success", "message": "ESP32 доступен"}), 200
        else:
            return jsonify({"status": "error", "message": f"ESP32 вернул код {resp.status_code}"}), 200
    except requests.exceptions.RequestException:
        # Если запрос не удался, значит ESP32 недоступен
        return jsonify({"status": "error", "message": "Не удалось достучаться до ESP32"}), 200

@app.route('/')
def index():
    return render_template('index.html')

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=False)
