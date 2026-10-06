
from flask import Flask, jsonify, send_from_directory
import serial
import threading
import time

app = Flask(__name__)

# Change COM3 to your Arduino's port
SERIAL_PORT = "COM5"
BAUD_RATE = 9600

# Latest GPS and sensor data
gps_data = {
    "latitude": None,
    "longitude": None,
    "impact": 0,
    "gps_status": "Waiting for GPS",
    "last_update": None
}

# Locations where potholes were detected
potholes = []

lock = threading.Lock()


def read_arduino():
    global gps_data, potholes

    try:
        arduino = serial.Serial(
            SERIAL_PORT, BAUD_RATE, timeout=1
        )

        # Allow Arduino to restart after serial connection
        time.sleep(2)
        print("Arduino connected:", SERIAL_PORT)

        previous_impact = 0

        while True:
            line = arduino.readline().decode(
                "utf-8", errors="ignore"
            ).strip()

            if not line:
                continue

            print("Arduino:", line)

            try:
                # Expected format:
                # latitude,longitude
                # OR latitude,longitude,impact
                values = line.split(",")

                if len(values) < 2:
                    continue

                latitude = float(values[0].strip())
                longitude = float(values[1].strip())

                impact = 0
                if len(values) >= 3:
                    impact = int(float(values[2].strip()))

                # Validate coordinates
                if not (-90 <= latitude <= 90):
                    continue

                if not (-180 <= longitude <= 180):
                    continue

                # Ignore invalid GPS fix
                if latitude == 0 and longitude == 0:
                    continue

                current_time = time.strftime(
                    "%Y-%m-%d %H:%M:%S"
                )

                with lock:
                    gps_data.update({
                        "latitude": latitude,
                        "longitude": longitude,
                        "impact": impact,
                        "gps_status": "GPS Connected",
                        "last_update": current_time
                    })

                    # Record a pothole when impact changes
                    # from 0 to 1.
                    if impact == 1 and previous_impact == 0:
                        potholes.append({
                            "latitude": latitude,
                            "longitude": longitude,
                            "time": current_time
                        })

                        print(
                            "POTHOLE DETECTED:",
                            latitude, longitude
                        )

                previous_impact = impact

            except (ValueError, IndexError):
                # Ignore text messages and incomplete lines
                continue

    except serial.SerialException as error:
        print("Serial connection error:", error)
        with lock:
            gps_data["gps_status"] = "Arduino disconnected"


@app.route("/")
def home():
    return send_from_directory(".", "index.html")


@app.route("/api/gps")
def get_gps():
    with lock:
        return jsonify(gps_data.copy())


@app.route("/api/potholes")
def get_potholes():
    with lock:
        return jsonify(list(potholes))


if __name__ == "__main__":
    # Read Arduino data in the background
    thread = threading.Thread(
        target=read_arduino, daemon=True
    )
    thread.start()

    # Start the webpage server
    app.run(
        host="127.0.0.1",
        port=5000,
        debug=False
    )