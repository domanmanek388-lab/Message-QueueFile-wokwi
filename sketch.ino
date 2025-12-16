#define LED1    12
#define LED2    13
#define Analog  32
#define BUTTON  0      
QueueHandle_t integerQueue;
volatile int blinkMode = 0;  // 0,1,2 lalu kembali ke 0

void setup() {

  Serial.begin(9600);
  while (!Serial) {
    delay(10);
  }

  pinMode(BUTTON, INPUT_PULLUP);

  integerQueue = xQueueCreate(1, sizeof(int));

  if (integerQueue != NULL) {
    xTaskCreate(TaskSerial,      "Serial",     1024, NULL, 2, NULL);
    xTaskCreate(TaskAnalogRead,  "AnalogRead", 1024, NULL, 1, NULL);
  }

  xTaskCreate(TaskBlink,  "Blink",  1024, NULL, 0, NULL);
  xTaskCreate(TaskButton, "Button", 1024, NULL, 1, NULL);
}


void loop() {
  vTaskDelete(NULL); 
}

// task baca analog 
void TaskAnalogRead(void *pvParameters) {
  (void) pvParameters;

  for (;;) {
    int sensorValue = analogRead(Analog);
    xQueueSend(integerQueue, &sensorValue, portMAX_DELAY);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

// task serial
void TaskSerial(void *pvParameters) {
  (void) pvParameters;

  int valueFromQueue = 0;

  for (;;) {
    if (xQueueReceive(integerQueue, &valueFromQueue, portMAX_DELAY) == pdPASS) {
      Serial.print("Data analog: ");
      Serial.println(valueFromQueue);
    }
  }
}

// task blink led
void TaskBlink(void *pvParameters) {
  (void) pvParameters;

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  bool state = true;

  for (;;) {
    int delayTime;

    switch (blinkMode) {
      case 0: delayTime = 1000; break; // 1 detik
      case 1: delayTime = 2000; break; // 2 detik
      case 2: delayTime = 500;  break; // 0.5 detik
      default: delayTime = 1000;
    }

    digitalWrite(LED1, state);
    digitalWrite(LED2, !state);
    state = !state;

    vTaskDelay(delayTime / portTICK_PERIOD_MS);
  }
}

// task tombol
void TaskButton(void *pvParameters) {
  (void) pvParameters;

  bool lastState = HIGH;
  bool currentState;

  for (;;) {
    currentState = digitalRead(BUTTON);

    // Deteksi tombol ditekan (HIGH → LOW)
    if (lastState == HIGH && currentState == LOW) {

      blinkMode++;
      if (blinkMode > 2) {
        blinkMode = 0;
      }

      Serial.print("Mode kedip berubah menjadi: ");
      Serial.println(blinkMode);

      vTaskDelay(200 / portTICK_PERIOD_MS); 
    }

    lastState = currentState;
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}
