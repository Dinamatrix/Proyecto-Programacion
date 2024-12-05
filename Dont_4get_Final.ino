#include <Wire.h>
#include <MFRC522.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>

#define RST_PIN 22  // Pin RST para el lector RFID
#define SS_PIN 5   // Pin SS para el lector RFID
#define SCAN_TIME 5 // Tiempo de escaneo BLE en segundos

int led = 13;
// Objetos para RFID
MFRC522 rfid(SS_PIN, RST_PIN);

// Variables para BLE
BLEScan* pBLEScan;
String targetDeviceName = "Nose";  // Nombre con el que el celular se debe presentar
int rssiThreshold = -50;  // Umbral RSSI para considerar que el celular está cerca

void setup() {
  Serial.begin(115200);

  pinMode(led, OUTPUT);

  // Inicialización del lector RFID
  SPI.begin();
  rfid.PCD_Init();
  Serial.println("RFID listo");

  // Inicialización BLE
  BLEDevice::init("ESP32_RFID_RSSI");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setActiveScan(true);  // Escaneo activo para obtener más datos
  Serial.println("BLE listo");
}

void loop() {
  // Escaneo BLE
  BLEScanResults scanResults = pBLEScan->start(SCAN_TIME, false);

  bool isPhoneNear = false; // Variable para detectar si el celular está cerca

  for (int i = 0; i < scanResults.getCount(); i++) {
    BLEAdvertisedDevice device = scanResults.getDevice(i);

    String deviceName = device.getName().c_str();  // Nombre del dispositivo detectado
    int rssiValue = device.getRSSI();  // Valor RSSI

    if (deviceName == targetDeviceName && rssiValue > rssiThreshold) {
      isPhoneNear = true;
      Serial.println("Celular detectado cerca.");
      break;  // No necesitamos seguir buscando
    }
  }

  pBLEScan->clearResults();  // Limpia los resultados del escaneo

  // Si el celular está cerca, activa el lector RFID
  if (isPhoneNear) {
    Serial.println("Activando lector RFID...");
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {

      //Activación de la alarma
      for (int i = 0; i < 4; i++){
        digitalWrite(led, HIGH);
        delay(500);
        digitalWrite(led,LOW);
        delay(500);
      }
      Serial.print("Etiqueta detectada. UID: ");
      for (byte i = 0; i < rfid.uid.size; i++) {
        Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(rfid.uid.uidByte[i], HEX);
      }
      Serial.println();
      delay(2000);  // Espera para evitar múltiples lecturas rápidas
    } else {
      Serial.println("No se detectaron etiquetas.");
    }
  } else {
    Serial.println("Celular no está cerca. Lector RFID desactivado.");
  }

  delay(1000); // Espera antes de la próxima iteración
}
