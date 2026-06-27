#include <Arduino.h>

// ---------------- Pin configuration ----------------
#define TEMP_PIN 34
#define VIB_PIN 35
#define LED_PIN 2

// Logic analyzer trace pins
#define TEMP_TRACE_PIN 13
#define VIB_TRACE_PIN 12
#define LOG_TRACE_PIN 14
#define LED_TRACE_PIN 27

// ---------------- Timing requirements ----------------
#define TEMP_PERIOD_MS 200
#define VIB_PERIOD_MS 500
#define LOG_PERIOD_MS 1000
#define SD_WRITE_TIME_MS 300
#define LED_TOGGLE_MS 250
#define TEMP_JITTER_LIMIT_MS 10

// ---------------- FreeRTOS priorities ----------------
#define TEMP_PRIORITY 5
#define VIB_PRIORITY 4
#define LOG_TRIGGER_PRIORITY 3
#define LOGGER_PRIORITY 2
#define STATS_PRIORITY 1
#define LED_PRIORITY 1

// ---------------- Queue data model ----------------
enum SensorType {
  SENSOR_TEMP,
  SENSOR_VIB
};

struct SensorRecord {
  SensorType type;
  int adcValue;
  unsigned long timestamp;
  long intervalMs;
  long jitterMs;
};

// ---------------- FreeRTOS handles ----------------
QueueHandle_t sensorQueue;
SemaphoreHandle_t logSemaphore;
SemaphoreHandle_t serialMutex;

// ---------------- Runtime statistics ----------------
volatile unsigned long tempSamples = 0;
volatile unsigned long vibSamples = 0;
volatile unsigned long tempDeadlineMisses = 0;
volatile unsigned long droppedQueueRecords = 0;
volatile long maxTempJitter = 0;
volatile long maxVibJitter = 0;

// ---------------- Helper: safe Serial print using Mutex ----------------
void safePrint(String message) {
  if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
    Serial.println(message);
    xSemaphoreGive(serialMutex);
  }
}

// ---------------- Update statistics ----------------
void updateStats(SensorType type, long jitterMs, bool queueOk) {
  long absJitter = abs(jitterMs);

  if (type == SENSOR_TEMP) {
    tempSamples++;

    if (absJitter > maxTempJitter) {
      maxTempJitter = absJitter;
    }

    if (absJitter > TEMP_JITTER_LIMIT_MS) {
      tempDeadlineMisses++;
    }
  } 
  else if (type == SENSOR_VIB) {
    vibSamples++;

    if (absJitter > maxVibJitter) {
      maxVibJitter = absJitter;
    }
  }

  if (!queueOk) {
    droppedQueueRecords++;
  }
}

// ---------------- Task 1: Temperature sensor ----------------
void temperatureTask(void *pvParameters) {
  TickType_t lastWakeTime = xTaskGetTickCount();
  unsigned long previousTime = 0;
  bool firstRun = true;

  while (1) {
    vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(TEMP_PERIOD_MS));

    digitalWrite(TEMP_TRACE_PIN, HIGH);

    unsigned long now = millis();
    long interval = firstRun ? TEMP_PERIOD_MS : now - previousTime;
    long jitter = interval - TEMP_PERIOD_MS;
    previousTime = now;
    firstRun = false;

    SensorRecord record;
    record.type = SENSOR_TEMP;
    record.adcValue = analogRead(TEMP_PIN);
    record.timestamp = now;
    record.intervalMs = interval;
    record.jitterMs = jitter;

    bool queueOk = (xQueueSend(sensorQueue, &record, 0) == pdTRUE);

    updateStats(SENSOR_TEMP, jitter, queueOk);

    digitalWrite(TEMP_TRACE_PIN, LOW);
  }
}

// ---------------- Task 2: Vibration sensor ----------------
void vibrationTask(void *pvParameters) {
  TickType_t lastWakeTime = xTaskGetTickCount();
  unsigned long previousTime = 0;
  bool firstRun = true;

  while (1) {
    vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(VIB_PERIOD_MS));

    digitalWrite(VIB_TRACE_PIN, HIGH);

    unsigned long now = millis();
    long interval = firstRun ? VIB_PERIOD_MS : now - previousTime;
    long jitter = interval - VIB_PERIOD_MS;
    previousTime = now;
    firstRun = false;

    SensorRecord record;
    record.type = SENSOR_VIB;
    record.adcValue = analogRead(VIB_PIN);
    record.timestamp = now;
    record.intervalMs = interval;
    record.jitterMs = jitter;

    bool queueOk = (xQueueSend(sensorQueue, &record, 0) == pdTRUE);

    updateStats(SENSOR_VIB, jitter, queueOk);

    digitalWrite(VIB_TRACE_PIN, LOW);
  }
}

// ---------------- Task 3: Logging trigger using Semaphore ----------------
void logTriggerTask(void *pvParameters) {
  TickType_t lastWakeTime = xTaskGetTickCount();

  while (1) {
    vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(LOG_PERIOD_MS));

    // Give semaphore every 1000 ms to trigger logger task
    xSemaphoreGive(logSemaphore);
  }
}

// ---------------- Task 4: Logger task ----------------
void loggingTask(void *pvParameters) {
  SensorRecord record;

  while (1) {
    // Wait until semaphore is given by logTriggerTask
    if (xSemaphoreTake(logSemaphore, portMAX_DELAY) == pdTRUE) {
      digitalWrite(LOG_TRACE_PIN, HIGH);

      if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        Serial.println();
        Serial.println("------ SD/UART LOGGING START ------");
        Serial.print("Logger triggered at: ");
        Serial.print(millis());
        Serial.println(" ms");
        Serial.println("Simulated SD write delay: 300 ms");
        xSemaphoreGive(serialMutex);
      }

      // Simulate slow SD card writing process
      vTaskDelay(pdMS_TO_TICKS(SD_WRITE_TIME_MS));

      int recordsLogged = 0;

      if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        Serial.println("Time(ms)\tSensor\tADC\tInterval\tJitter");

        while (xQueueReceive(sensorQueue, &record, 0) == pdTRUE) {
          Serial.print(record.timestamp);
          Serial.print("\t\t");

          if (record.type == SENSOR_TEMP) {
            Serial.print("TEMP");
          } else {
            Serial.print("VIB");
          }

          Serial.print("\t");
          Serial.print(record.adcValue);
          Serial.print("\t");
          Serial.print(record.intervalMs);
          Serial.print(" ms\t\t");
          Serial.print(record.jitterMs);
          Serial.println(" ms");

          recordsLogged++;
        }

        Serial.print("Records logged: ");
        Serial.println(recordsLogged);
        Serial.println("------ SD/UART LOGGING END --------");
        xSemaphoreGive(serialMutex);
      }

      digitalWrite(LOG_TRACE_PIN, LOW);
    }
  }
}

// ---------------- Task 5: Runtime statistics ----------------
void statsTask(void *pvParameters) {
  while (1) {
    vTaskDelay(pdMS_TO_TICKS(2000));

    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      Serial.println();
      Serial.println("[STATS] Runtime Statistics");
      Serial.print("Temperature samples      : ");
      Serial.println(tempSamples);
      Serial.print("Vibration samples        : ");
      Serial.println(vibSamples);
      Serial.print("Max temperature jitter   : ");
      Serial.print(maxTempJitter);
      Serial.println(" ms");
      Serial.print("Max vibration jitter     : ");
      Serial.print(maxVibJitter);
      Serial.println(" ms");
      Serial.print("Temperature deadline miss: ");
      Serial.println(tempDeadlineMisses);
      Serial.print("Queue records dropped    : ");
      Serial.println(droppedQueueRecords);
      Serial.print("Queue spaces available   : ");
      Serial.println(uxQueueSpacesAvailable(sensorQueue));
      xSemaphoreGive(serialMutex);
    }
  }
}

// ---------------- Task 6: LED alive task ----------------
void ledTask(void *pvParameters) {
  TickType_t lastWakeTime = xTaskGetTickCount();

  while (1) {
    digitalWrite(LED_TRACE_PIN, HIGH);

    // Toggle LED
    bool ledState = !digitalRead(LED_PIN);
    digitalWrite(LED_PIN, ledState);

    // Print LED status and blink frequency
    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(20)) == pdTRUE) {
      float blinkHz = 1000.0 / (2.0 * LED_TOGGLE_MS);

      Serial.print("[LED Task] LED ");
      Serial.print(ledState ? "ON" : "OFF");
      Serial.print(" | Blink Frequency = ");
      Serial.print(blinkHz, 1);
      Serial.println(" Hz");

      xSemaphoreGive(serialMutex);
    }

    vTaskDelay(pdMS_TO_TICKS(20));

    digitalWrite(LED_TRACE_PIN, LOW);

    vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(LED_TOGGLE_MS));
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);

  pinMode(TEMP_TRACE_PIN, OUTPUT);
  pinMode(VIB_TRACE_PIN, OUTPUT);
  pinMode(LOG_TRACE_PIN, OUTPUT);
  pinMode(LED_TRACE_PIN, OUTPUT);

  analogReadResolution(12);

  // ---------------- Create FreeRTOS objects ----------------
  sensorQueue = xQueueCreate(32, sizeof(SensorRecord));
  logSemaphore = xSemaphoreCreateBinary();
  serialMutex = xSemaphoreCreateMutex();

  if (sensorQueue == NULL || logSemaphore == NULL || serialMutex == NULL) {
    Serial.println("FreeRTOS object creation failed.");
    while (1);
  }

  Serial.println("FreeRTOS Predictive Maintenance System Started");
  Serial.println("Member 4 Implementation: Queue, Semaphore, Mutex, Testing and Runtime Statistics");
  Serial.println("Temperature Task: 200 ms, highest priority");
  Serial.println("Vibration Task: 500 ms, medium priority");
  Serial.println("Logger Task: 1000 ms, simulated 300 ms SD write");
  Serial.println("LED Task: 2 Hz alive indicator");

  // ---------------- Create tasks ----------------
  xTaskCreatePinnedToCore(temperatureTask, "Temperature Task", 4096, NULL, TEMP_PRIORITY, NULL, 1);
  xTaskCreatePinnedToCore(vibrationTask, "Vibration Task", 4096, NULL, VIB_PRIORITY, NULL, 1);
  xTaskCreatePinnedToCore(logTriggerTask, "Log Trigger Task", 4096, NULL, LOG_TRIGGER_PRIORITY, NULL, 1);
  xTaskCreatePinnedToCore(loggingTask, "Logging Task", 4096, NULL, LOGGER_PRIORITY, NULL, 1);
  xTaskCreatePinnedToCore(statsTask, "Stats Task", 4096, NULL, STATS_PRIORITY, NULL, 1);
  xTaskCreatePinnedToCore(ledTask, "LED Task", 2048, NULL, LED_PRIORITY, NULL, 1);
}

void loop() {
  // Empty because all work is handled by FreeRTOS tasks
}