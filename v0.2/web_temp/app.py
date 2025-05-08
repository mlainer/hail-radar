from flask import Flask, request, render_template, jsonify
import mysql.connector
from datetime import datetime

app = Flask(__name__)

def get_db_connection():
    conn = mysql.connector.connect(
        host="db",
        database="temperature_db",
        user="user",
        password="password"
    )
    return conn

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/data')
def data():
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute('SELECT timestamp, temp_c, temp_f FROM temperature ORDER BY timestamp')
    rows = cur.fetchall()
    cur.close()
    conn.close()
    # Convert data to a format suitable for Chart.js
    data = {
        "labels": [row[0].strftime("%Y-%m-%d %H:%M:%S") for row in rows],
        "datasets": [
            {
                "label": "Temperature (°C)",
                "data": [row[1] for row in rows],
                "borderColor": "rgba(75, 192, 192, 1)",
                "fill": False
            },
            {
                "label": "Temperature (°F)",
                "data": [row[2] for row in rows],
                "borderColor": "rgba(255, 99, 132, 1)",
                "fill": False
            }
        ]
    }
    return jsonify(data)

if __name__ == '__main__':
    app.run(host='0.0.0.0')

