from flask import Flask, render_template
from flask_socketio import SocketIO
import mysql.connector
from datetime import datetime, timedelta
import threading
import time

app = Flask(__name__)
socketio = SocketIO(app)

# MySQL Database Configuration
db_config = {
    'user': 'user',
    'password': 'password',
    'host': 'localhost',
    'database': 'hail_speed_db',
}

def get_max_speed():
    connection = mysql.connector.connect(**db_config)
    cursor = connection.cursor()
    query = """
        SELECT MAX(speed)
        FROM hail_speed_data
        WHERE time >= %s
    """
    seconds_ago = datetime.utcnow() - timedelta(seconds=3)
    cursor.execute(query, (seconds_ago.timestamp(),))
    result = cursor.fetchone()
    connection.close()
    #speed_out = "%.2f" % round(result[0],2)
    return result[0] if result[0] is not None else 0

def background_thread():
    while True:
        max_speed = get_max_speed()
        socketio.emit('update_speed', {'max_speed': max_speed})
        time.sleep(1)

@app.route('/')
def index():
    return render_template('index.html')

@socketio.on('connect')
def handle_connect():
    max_speed = get_max_speed()
    socketio.emit('update_speed', {'max_speed': max_speed})

if __name__ == '__main__':
    thread = threading.Thread(target=background_thread)
    thread.daemon = True
    thread.start()
    socketio.run(app, host='0.0.0.0', port=5005)

