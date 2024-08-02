/*
  Project: Ultrasonic Sensor Distance Measurement and Data Logging to SD Card
  Description: This project measures the distance to an object using an ultrasonic sensor and logs the data to an SD card in CSV format.
               The measurements are taken at regular intervals and stored in multiple files, each representing a 10-second recording segment.
               The total recording duration is set to 1 minute.

  How It Works:
  - The Arduino sends an ultrasonic pulse using the trigger pin of the ultrasonic sensor.
  - The echo pin of the sensor receives the reflected pulse and calculates the duration it took for the pulse to return.
  - This duration is then used to calculate the distance to the object.
  - The distance measurements are recorded along with the elapsed time into CSV files on an SD card.
  - A new CSV file is created every 10 seconds to segment the data.

  Hardware Connections:
  - Ultrasonic Sensor:
    - VCC to 5V on Arduino Mega 2560
    - GND to GND on Arduino Mega 2560
    - Trig pin to digital pin 9 on Arduino Mega 2560
    - Echo pin to digital pin 10 on Arduino Mega 2560
  - SD Card Module:
    - VCC to 5V on Arduino Mega 2560
    - GND to GND on Arduino Mega 2560
    - CS (Chip Select) to pin 53 on Arduino Mega 2560
    - MOSI to pin 51 on Arduino Mega 2560
    - MISO to pin 50 on Arduino Mega 2560
    - SCK to pin 52 on Arduino Mega 2560
*/

#include <SPI.h> // SPI library for communication with the SD card
#include <SD.h>  // SD library to handle SD card operations

// Define pins for the ultrasonic sensor
const int trigPin = 9;  // Trigger pin for the ultrasonic sensor
const int echoPin = 10; // Echo pin for the ultrasonic sensor

// Define variables for duration and distance measurement
long duration; // Variable to store the time it takes for the ultrasonic pulse to return
int distance;  // Variable to store the calculated distance based on the duration

// Define chip select pin for the SD card adapter
const int chipSelect = 53; // Chip select pin for the SD card module

// File object for the SD card
File myFile; // Object to handle file operations on the SD card

// Define the recording duration (in milliseconds)
const long totalRecordDuration = 60000; // Total recording duration: 1 minute
const long fileDuration = 10000; // Duration for each file: 10 seconds

// Variables to store the start time of the recording and the start time of each file
unsigned long startTime;     // Time when the recording started
unsigned long fileStartTime; // Time when the current file recording started

// Counter for file naming
int fileCounter = 1; // Counter to create unique file names for each 10-second segment

void setup() {
  // Initialize the serial monitor for debugging purposes
  Serial.begin(9600);

  // Set the trigPin as an output
  pinMode(trigPin, OUTPUT);
  
  // Set the echoPin as an input
  pinMode(echoPin, INPUT);

  // Initialize the SD card
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed!"); // Print error message if SD card initialization fails
    while (1); // Halt execution if SD card initialization fails
  }
  Serial.println("SD card is ready to use."); // Print success message if SD card initialization succeeds

  // Record the start time of the recording and the start time for the first file
  startTime = millis();
  fileStartTime = startTime;

  // Create the first file for data logging
  createNewFile();
}

void loop() {
  // Calculate the current time and the elapsed time since the start of the recording
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - startTime;
  unsigned long fileElapsedTime = currentTime - fileStartTime;

  // Check if the total recording duration has not been exceeded
  if (elapsedTime < totalRecordDuration) {
    // Check if it's time to create a new file (every 10 seconds)
    if (fileElapsedTime >= fileDuration) {
      fileCounter++; // Increment the file counter
      fileStartTime = currentTime; // Update the start time for the new file
      createNewFile(); // Create a new file for the next 10-second segment
    }

    // Clear the trigPin by setting it LOW for 2 microseconds
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    
    // Set the trigPin HIGH for 10 microseconds to send an ultrasonic pulse
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Read the echoPin, which returns the sound wave travel time in microseconds
    duration = pulseIn(echoPin, HIGH);

    // Calculate the distance based on the duration
    distance = duration * 0.034 / 2;

    // Print the elapsed time and distance to the Serial Monitor
    Serial.print("Elapsed Time: ");
    Serial.print(elapsedTime / 1000.0); // Convert milliseconds to seconds
    Serial.print(" s, Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    // Open the current CSV file for writing
    char fileName[15];
    sprintf(fileName, "datalog%d.csv", fileCounter); // Create a unique file name
    myFile = SD.open(fileName, FILE_WRITE);
    if (myFile) {
      // Record the timestamp (in seconds) and distance to the CSV file
      myFile.print(elapsedTime / 1000.0); // Convert milliseconds to seconds
      myFile.print(",");
      myFile.println(distance);
      myFile.close(); // Close the file to ensure data is saved
      Serial.print("Recorded to "); // Print message indicating data recording
      Serial.println(fileName);
    } else {
      Serial.println("Error opening file"); // Print error message if file cannot be opened
    }

    // Wait for a short period (500 milliseconds) before the next measurement
    delay(500);
  } else {
    // Stop the loop after 1 minute of recording
    Serial.println("Recording complete.");
    while (1); // Halt execution after recording is complete
  }
}

// Function to create a new CSV file and write the header
void createNewFile() {
  char fileName[15];
  sprintf(fileName, "datalog%d.csv", fileCounter); // Create a unique file name
  myFile = SD.open(fileName, FILE_WRITE);
  if (myFile) {
    // Write the header to the new file
    myFile.println("Timestamp (s),Distance (cm)");
    myFile.close(); // Close the file to ensure data is saved
    Serial.print("Created ");
    Serial.println(fileName); // Print message indicating file creation
  } else {
    Serial.println("Error creating file"); // Print error message if file cannot be created
  }
}

