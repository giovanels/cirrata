#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <WebServer.h>

//Credenciais Wi-Fi
const char* ssid = "Giovane";
const char* password = "12345678";

//Webserver na porta 80
WebServer server(80);

String dados = "Aguardando dados...";

//Pagina HTML com atualização a cada 2 segundos
void handleRoot() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta http-equiv='refresh' content='2'>"; // Recarrega a cada 2 segundos
  html += "<meta charset='utf-8'><title>Dados LoRa</title></head><body>";
  html += "<h2>Último dado recebido via LoRa:</h2>";
  html += "<p style='font-size:24px;'>" + dados + "</p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  //Conecta ao Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  //Inicia servidor web
  server.on("/", handleRoot);
  server.begin();
  Serial.println("Servidor web iniciado!");

  //Inicializa o LoRa
  LoRa.setPins(18, 14, 26); // NSS, RST, DIO0
  if (!LoRa.begin(915E6)) {
    Serial.println("Erro ao iniciar LoRa");
    while (1);
  }
  Serial.println("LoRa iniciado com sucesso!");
}

void loop() {
  //Checa se há pacote via LoRa
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String recebido = "";
    while (LoRa.available()) {
      recebido += (char)LoRa.read();
    }
    dados = recebido;
    Serial.print("Recebido via LoRa: ");
    Serial.println(dados);
  }

  server.handleClient();
  delay(10);
}
