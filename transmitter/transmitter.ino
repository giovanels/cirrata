#include <LoRa.h>
void setup() {
  Serial.begin(115200);
  LoRa.setPins(18, 14, 26);
  if (!LoRa.begin(915E6)) {
    Serial.println("Erro ao iniciar LoRa");
    while (1);
  }
}

void loop() {
  char buff[2] = { 0 };

  // put your main code here, to run repeatedly:
  if(Serial.available() > 0) {

    Serial.readBytes(buff, 2);

    Serial.printf("%#010x\n", buff);

    LoRa.beginPacket();
    LoRa.print(buff);
    LoRa.endPacket();

    Serial.println("Enviado via LoRa!");
  }

  delay(512);
}
