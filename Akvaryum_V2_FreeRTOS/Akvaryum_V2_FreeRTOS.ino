#include <Arduino_FreeRTOS.h>
#include "voltage.h"
#include "temp.h"
#include "RTC.h"

#define SET_BIT(reg, bit) ((reg) |= (1UL << (bit)))
#define CLEAR_BIT(reg, bit) ((reg) &= ~(1UL << (bit)))
#define SET_OUTPUT_REGISTER_BIT(reg, pin) (reg |= (1 << (pin)))
#define SET_INPUT_REGISTER_BIT(reg, pin) ((reg) &= ~(1 << (pin)))

#define BLT_state 22

// Define tasks
void processSerialCommand(void *pvParameters);
void measureTemperature(void *pvParameters);
void measureDateTime(void *pvParameters);
void connectionControl(void *pvParameters);

// Task handles
TaskHandle_t xProcessSerialCommand;
TaskHandle_t xMeasureTemperature;
TaskHandle_t xConnectionControl;

// Global variables for task periods
unsigned long temperatureTaskPeriod = 120000;  // Default period: 100 seconds
unsigned long dateTimeTaskPeriod = 60000;      // Default period: 60 seconds

// Default olarak sıcaklığı Celsius cinsinden göster
bool useFahrenheit = false;

bool last_blt_state = false;

// Global değişkenler
int hour, minute, second, day, month, year;

// Zaman bilgisini taşıyan string
String time_s = "", date_s = "";

//saat gösterimi default olarak aktif
bool show_time = true;

bool time_edit = false;

// Global variables for measured data
float measuredTemperature = 0.0, measuredTempOut = 0.0;
DateTime measuredDateTime;

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);
  Serial3.begin(115200);
  rtcBaslat();

  SET_OUTPUT_REGISTER_BIT(DDRA, 7);
  SET_OUTPUT_REGISTER_BIT(DDRC, 7);

  pinMode(BLT_state, INPUT);

  // Alttaki satır Arduino IDE'den gelen saat ve tarihi alarak RTC'yi ayarlar
  // Bu halde bir defalığına Arduino'ya yükledikten sonra satırı başına //
  // koyarak yorum yapıp tekrar yüklemeliyiz.
  // Aksi halde her açılışta tekrar saat yüklemeye çalışır.
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  // Create tasks
  xTaskCreate(processSerialCommand, "ProcessSerialCommand", 1024, NULL, 3, &xProcessSerialCommand);
  xTaskCreate(measureTemperature, "MeasureTemperature", 256, NULL, 1, &xMeasureTemperature);
  xTaskCreate(connectionControl, "ConnectionControl", 256, NULL, 4, &xConnectionControl);

  // Start scheduler
  vTaskStartScheduler();
}

void loop() {
  // Not used with FreeRTOS
}

void processSerialCommand(void *pvParameters) {
  String incomingString;  // Gelen seri port verisini saklamak için String nesnesi
  for (;;) {
    if (Serial.available() > 0) {                     // Serial porttan veri var mı kontrol edin
      incomingString = Serial.readStringUntil('\n');  // Serial porttan gelen String veriyi okuyun
      // Seri porttan gelen veri için işlem yapın
      incomingString.trim();
      processCommand(incomingString, Serial);
    }
    if (Serial2.available() > 0) {                     // Serial2 porttan veri var mı kontrol edin
      incomingString = Serial2.readStringUntil('\n');  // Serial2 porttan gelen String veriyi okuyun
      // Serial2 porttan gelen veri için işlem yapın
      incomingString.trim();
      processCommand(incomingString, Serial2);
    }
    if (Serial3.available() > 0) {                     // Serial3 porttan veri var mı kontrol edin
      incomingString = Serial3.readStringUntil('\n');  // Serial3 porttan gelen String veriyi okuyun
      // Serial3 porttan gelen veri için işlem yapın
      incomingString.trim();
      processCommand(incomingString, Serial3);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);  // Görevin her döngüsünde kısa bir bekleme
  }
}

void measureTemperature(void *pvParameters) {
  for (;;) {
    vTaskDelay(4000 / portTICK_PERIOD_MS);
    temp_update();
    // Seri porta ölçülen sıcaklık bilgilerini yazdır
    String m = printTime() + "(*) Sicaklik bilgisi guncellendi...";
    streamAll(m);
    // Belirlenen periyotta tekrar ölçüm yapmak için bekle
    vTaskDelay((temperatureTaskPeriod - 4000) / portTICK_PERIOD_MS);
  }
}

void connectionControl(void *pvParameters) {
  for (;;) {
    if (digitalRead(BLT_state)) {
      SET_BIT(PORTC, 7);
      vTaskDelay(40 / portTICK_PERIOD_MS);
      CLEAR_BIT(PORTC, 7);
    }
    else {
      CLEAR_BIT(PORTC, 7);
    }
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}

void processCommand(String command, Stream &stream) {
  SET_BIT(PORTA, 7);
  stream.println("");
  stream.print("+ ");
  stream.println(command);
  if (command.startsWith("tempPeriod")) {
    // Sıcaklık görevi periyodunu güncelle
    temperatureTaskPeriod = command.substring(11).toInt();
    stream.print(printTime() + "* Sicaklik task periyodu guncellendi: ");
    stream.println(temperatureTaskPeriod);
  } else if (command.startsWith("dateTimePeriod")) {
    // Tarih/saat görevi periyodunu güncelle
    dateTimeTaskPeriod = command.substring(15).toInt();
    stream.print(printTime() + "* Zaman task periyodu guncellendi: ");
    stream.println(dateTimeTaskPeriod);
  } else if (command.startsWith("debugTime")) {
    String unit = command.substring(9);
    unit.trim();
    if (unit == "1") {
      show_time = true;
      stream.println(printTime() + "Saat bilgisi gosterilecek.");
    } else if (unit == "0") {
      show_time = false;
      stream.println(printTime() + "Saat bilgisi gizlenecek.");
    } else {
      stream.println(printTime() + "Gecersiz birim. Lutfen '1' veya '0' girin.");
    }
  } else if (command.startsWith("led")) {
    String unit = command.substring(3);
    unit.trim();
    if (unit == "1") {
      SET_BIT(PORTA, 7);
      return;
    } else if (unit == "0") {
      CLEAR_BIT(PORTA, 7);
    } else {
      stream.println(printTime() + "Gecersiz birim. Lutfen '1' veya '0' girin.");
    }
  } else if (command == "time") {
    // Seri porta ölçülen tarih/saat bilgisini yazdırın
    update_rtc();
    stream.print("* Tarih: ");
    stream.println(date_s);
    stream.print("  Saat: ");
    stream.println(time_s);
  } else if (command == "voltage") {
    // Voltajları belirtilen akış nesnesine yazdır
    stream.println(printTime() + "Voltaj okumaları:");
    stream.print("Giris Voltaji: ");
    stream.println((String)voltageRead(SOURCE) + " V");
    stream.print("Ana Regulator Voltaji: ");
    stream.println((String)voltageRead(MAIN_REGULATOR) + " V");
    stream.print("ESP Regulator Voltaji: ");
    stream.println((String)voltageRead(ESP_REGULATOR) + " V");
    stream.print("RTC Pili Voltaji: ");
    stream.println((String)voltageRead(RTC_BAT) + " V");
  } else if (command == "temp") {
    // Seri porta ölçülen sıcaklık bilgisini yazdırın
    stream.print(printTime() + "* Sicaklik: ");
    stream.print(measuredTemperature);
    stream.println(useFahrenheit ? " 'F" : " 'C");
  } else if (command == "periods") {
    // Bütün taskleri ve periyodlarını özetleyin
    stream.println(printTime() + "* Task Periyotlari:");
    stream.print("  -> Sicaklik: ");
    stream.print(temperatureTaskPeriod);
    stream.println(" ms");
    stream.print("  -> Tarih: ");
    stream.print(dateTimeTaskPeriod);
    stream.println(" ms");
  } else if (command == "help") {
    // Yardım menüsünü yazdırın
    stream.println(printTime() + "* Available commands:");
    stream.println("->  tempPeriod [period] - Sets the temperature measurement period in milliseconds");
    stream.println("->  dateTimePeriod [period] - Sets the date/time measurement period in milliseconds");
    stream.println("->  time - Prints the current date and time");
    stream.println("->  temp - Prints the current temperature");
    stream.println("->  tempUnit [unit] - Sets the temperature unit to Fahrenheit ('F') or Celsius ('C')");
    stream.println("->  debugTime [unit] - Prints debug messages with timestamp. ('1') or ('0')");
    stream.println("->  led [state] - On and Off led. ('1') or ('0')");
    stream.println("->  periods - Prints the current task periods");
    stream.println("->  voltage - Measures voltages and prints them to the screen");
    stream.println("->  setDateTime [hr:min:sec dd/mm/yyyy] - Used to set the time and date");
  } else if (command.startsWith("tempUnit ")) {
    // Komutun ardından gelen karakterlerin alınması
    String unit = command.substring(9);
    if (unit == "F") {
      // Sıcaklık birimini Fahrenheit olarak ayarla
      useFahrenheit = true;
      stream.println(printTime() + "Sicaklik birimi Fahrenheit olarak ayarlandi.");
      temp_update();
    } else if (unit == "C") {
      // Sıcaklık birimini Celsius olarak ayarla
      useFahrenheit = false;
      stream.println(printTime() + "Sicaklik birimi Celsius olarak ayarlandi.");
      temp_update();
    } else {
      stream.println(printTime() + "Gecersiz birim. Lütfen 'F' veya 'C' girin.");
    }
  } else if (command.startsWith("setDateTime ")) {
    // Seri monitörden alınan saat ve tarih bilgisi
    String dateTimeString = command.substring(13);  // "setDateTime " kısmını atlıyoruz
    // Saat ve tarih bilgilerini ayrıştırma
    hour = dateTimeString.substring(0, 2).toInt();
    minute = dateTimeString.substring(3, 5).toInt();
    second = dateTimeString.substring(6, 8).toInt();
    day = dateTimeString.substring(9, 11).toInt();
    month = dateTimeString.substring(12, 14).toInt();
    year = dateTimeString.substring(15, 19).toInt();
    // Saat ve tarih bilgilerini yazdırma
    stream.print("Saat ve tarih bilgileri: ");
    stream.print(hour);
    stream.print(":");
    stream.print(minute);
    stream.print(":");
    stream.print(second);
    stream.print(" ");
    stream.print(day);
    stream.print("/");
    stream.print(month);
    stream.print("/");
    stream.println(year);
    time_edit = HIGH;
    // Onay isteme
    stream.println("Yukarıdaki bilgileri onaylıyor musunuz? (Evet/Hayir)");
  } else if (command == "Evet" && time_edit) {
    time_edit = LOW;
    // RTC'yi ayarlama işlemi (Onay alındığında)
    if ((year != 0) && (month != 0) && (day != 0)) {
      rtc.adjust(DateTime(year, month, day, hour, minute, second));
      update_rtc();
      // Onay mesajını gönderme
      stream.println("Saat ve tarih ayarlandı.");
    } else {
      stream.println("HATA: Saat ve tarih ayarlanamadı.");
    }
  } else if (command == "Hayir" && time_edit) {
    hour = 0;
    minute = 0;
    second = 0;
    day = 0;
    month = 0;
    year = 0;
    time_edit = LOW;
    // İptal mesajını gönderme
    stream.println("Saat ve tarih ayarlanmadı. İşlem iptal edildi.");
  } else {
    // Bilinmeyen komut için bir cevap gönderin
    stream.println(printTime() + "** Bilinmeyen komut.");
  }
  vTaskDelay(40 / portTICK_PERIOD_MS);
  CLEAR_BIT(PORTA, 7);
}

String printTime() {
  if (show_time) {
    update_rtc();
    return (time_s + " -> ");
  }
  return "";
}

template<typename T>
void streamAll(T message) {
  SET_BIT(PORTA, 7);
  Serial.println(message);
  Serial2.println(message);
  if (digitalRead(BLT_state))Serial3.println(message);
  vTaskDelay(20 / portTICK_PERIOD_MS);
  CLEAR_BIT(PORTA, 7);
}

String p2digit(int deger) {
  String t = (String)deger;
  if (deger < 10) {
    t = "0" + (String)deger;
  }
  return t;
}

void temp_update() {
  measuredTemperature = Temp(0, (useFahrenheit ? 1 : 0));  // İç ortam sıcaklığını ölç
}

void update_rtc() {
  measuredDateTime = rtc.now();
  time_s = p2digit(measuredDateTime.hour()) + ":" + p2digit(measuredDateTime.minute()) + ":" + p2digit(measuredDateTime.second());
  date_s = p2digit(measuredDateTime.day()) + "/" + p2digit(measuredDateTime.month()) + "/" + p2digit(measuredDateTime.year());
}
