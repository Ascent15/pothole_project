#include <Wire.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <MPU6050_light.h>

// -----------------------------
// GPS
// -----------------------------

TinyGPSPlus gps;

// NEO-6M TX -> Arduino D4
// NEO-6M RX -> Arduino D3
SoftwareSerial gpsSerial(4, 3);


// -----------------------------
// MPU6050
// -----------------------------

MPU6050 mpu(Wire);


// -----------------------------
// Timing
// -----------------------------

unsigned long startTime;


// -----------------------------
// Setup
// -----------------------------

void setup()
{
  Serial.begin(9600);

  gpsSerial.begin(9600);

  Wire.begin();


  // Start MPU6050
  byte status = mpu.begin();

  if (status != 0)
  {
    Serial.println("MPU6050 connection failed!");

    while (1)
    {
      delay(100);
    }
  }


  Serial.println("MPU6050 connected");

  delay(1000);


  // Calibrate MPU6050
  Serial.println("Calibrating MPU6050...");

  mpu.calcOffsets();

  Serial.println("Calibration complete");


  Serial.println();
  Serial.println("==============================");
  Serial.println("FIRST 5 SECONDS: MPU6050 DATA");
  Serial.println("==============================");


  startTime = millis();
}


// -----------------------------
// Main Loop
// -----------------------------

void loop()
{
  // -----------------------------
  // Always read GPS data
  // -----------------------------

  while (gpsSerial.available() > 0)
  {
    gps.encode(
      gpsSerial.read()
    );
  }


  // -----------------------------
  // Update MPU6050
  // -----------------------------

  mpu.update();


  // -----------------------------
  // First 5 seconds
  // -----------------------------

  if (millis() - startTime < 5000)
  {
    static unsigned long lastMPUPrint = 0;


    if (millis() - lastMPUPrint >= 200)
    {
      lastMPUPrint = millis();


      // Acceleration
      float ax = mpu.getAccX();
      float ay = mpu.getAccY();
      float az = mpu.getAccZ();


      // Gyroscope
      float gx = mpu.getGyroX();
      float gy = mpu.getGyroY();
      float gz = mpu.getGyroZ();


      // Total acceleration
      float totalAcceleration = sqrt(
        (ax * ax) +
        (ay * ay) +
        (az * az)
      );


      Serial.println("-----------------------------");

      Serial.print("Acceleration X: ");
      Serial.print(ax, 2);
      Serial.println(" g");

      Serial.print("Acceleration Y: ");
      Serial.print(ay, 2);
      Serial.println(" g");

      Serial.print("Acceleration Z: ");
      Serial.print(az, 2);
      Serial.println(" g");

      Serial.print("Total Acceleration: ");
      Serial.print(totalAcceleration, 2);
      Serial.println(" g");


      Serial.print("Gyro X: ");
      Serial.print(gx, 2);
      Serial.println(" deg/s");

      Serial.print("Gyro Y: ");
      Serial.print(gy, 2);
      Serial.println(" deg/s");

      Serial.print("Gyro Z: ");
      Serial.print(gz, 2);
      Serial.println(" deg/s");
    }


    return;
  }


  // -----------------------------
  // After 5 seconds
  // GPS status
  // -----------------------------

  static unsigned long lastGPSPrint = 0;


  if (millis() - lastGPSPrint >= 1000)
  {
    lastGPSPrint = millis();


    Serial.println();
    Serial.println("=============================");

    Serial.print("Satellites: ");

    if (gps.satellites.isValid())
    {
      Serial.println(
        gps.satellites.value()
      );
    }
    else
    {
      Serial.println("Searching...");
    }


    // Latitude
    Serial.print("Latitude: ");

    if (gps.location.isValid())
    {
      Serial.println(
        gps.location.lat(),
        6
      );
    }
    else
    {
      Serial.println("No Fix");
    }


    // Longitude
    Serial.print("Longitude: ");

    if (gps.location.isValid())
    {
      Serial.println(
        gps.location.lng(),
        6
      );
    }
    else
    {
      Serial.println("No Fix");
    }


    // GPS status
    Serial.print("GPS Status: ");

    if (
      gps.location.isValid() &&
      gps.satellites.isValid() &&
      gps.satellites.value() >= 4
    )
    {
      Serial.println("GPS FIX OK");
    }
    else
    {
      Serial.println("SEARCHING FOR SATELLITES");
    }

    Serial.println("=============================");
  }
}