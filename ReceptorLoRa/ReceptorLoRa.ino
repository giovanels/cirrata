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

unsigned int rpm;
unsigned int velo;
unsigned int cvt_temp;
String comb;

unsigned long pkgs_recv = 0;

void handleRoot() {
  String html =
    "<!DOCTYPE html>"
    "<html lang=\"pt-BR\">"
    "<head>"
    "<meta http-equiv='refresh' content='0.5'>"
    "  <meta charset=\"UTF-8\" />"
    "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />"
    "  <title>Telemetria LoRa Baja Guaicurus</title>"
    "  <style>"
    "    body {"
    "      background-color: #740712;"
    "      color: #f8f8f8;"
    "      font-family: 'Segoe UI', sans-serif;"
    "      text-align: center;"
    "      padding: 40px;"
    "    }"
    "    h1 {"
    "      font-size: 2.2rem;"
    "      color: #ffffff;"
    "      margin-bottom: 30px;"
    "    }"
    "    .info-box {"
    "      background-color: #1e1f2f;"
    "      padding: 20px;"
    "      border-radius: 15px;"
    "      margin: 15px auto;"
    "      max-width: 400px;"
    "      box-shadow: 0 0 12px rgba(0,0,0,0.5);"
    "    }"
    "    .label {"
    "      font-size: 1.2rem;"
    "      color: #f7c8d1;"
    "    }"
    "    .value {"
    "      font-size: 2rem;"
    "      font-weight: bold;"
    "      margin-top: 5px;"
    "    }"
    "    .footer {"
    "      margin-top: 40px;"
    "      font-size: 0.9rem;"
    "      color: #ddd;"
    "    }"
    "</style>"
    "</head>"
    "<body>"
    "  <h1>Telemetria LoRa Baja Guaicurus</h1>"
    "  <div class=\"info-box\">"
    "    <div class=\"label\">RPM:</div>"
    "    <div class=\"value\" id=\"rpm\">"
    + String(rpm) + " </div>"
                    "  </div>"
                    "  <div class=\"info-box\">"
                    "    <div class=\"label\">Velocidade:</div>"
                    "    <div class=\"value\" id=\"velocidade\">"
    + String(velo) + " km/h</div>"
                     "  </div>"
                     "  <div class=\"info-box\">"
                     "    <div class=\"label\">Temperatura da CVT:</div>"
                     "    <div class=\"value\" id=\"temp\">"
    + String(cvt_temp) + " °C</div>"
                         "  </div>"
                         "  <div class=\"info-box\">"
                         "    <div class=\"label\">Combustível:</div>"
                         "    <div class=\"value\" id=\"comb\">"
    + comb + "</div>"
             "  </div>"
             "  <div class=\"info-box\">"
             "    <div class=\"label\">Pacotes Recebidos: </div>"
             "    <div class=\"value\" id=\"packets_recv\">"
    + String(pkgs_recv) + "</div>"
                          "  </div>"
                          "  <div class=\"footer\">INFORMAÇÕES RECEBIDAS VIA LORA</div>"
                          "</body>"
                          "</html>";

  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

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
  LoRa.setPins(18, 14, 26);  // NSS, RST, DIO0
  if (!LoRa.begin(915E6)) {
    Serial.println("Erro ao iniciar LoRa");
    while (1)
      ;
  }
  Serial.println("LoRa iniciado com sucesso!");
}

void loop() {
  char buff[16] = { 0 };
  //Checa se há pacote via LoRa
  int packetSize = LoRa.parsePacket();

  if (packetSize == 11) {
    pkgs_recv++;

    for (uint8_t i = 0; i < packetSize; i++) {
      buff[i] = LoRa.read();
      Serial.printf("%c", buff[i]);
    }

    char tmp_conversor[8] = { 0 };

    // RPM
    snprintf(tmp_conversor, 8, "%c%c%c", buff[2], buff[3], buff[4]);
    rpm = strtol(tmp_conversor, NULL, 16);

    snprintf(tmp_conversor, 8, "%c%c", buff[5], buff[6]);
    velo = strtol(tmp_conversor, NULL, 16);

    snprintf(tmp_conversor, 8, "%c%c", buff[7], buff[8]);
    cvt_temp = strtol(tmp_conversor, NULL, 16);

    snprintf(tmp_conversor, 8, "%c", buff[9]);
    comb = atoi(tmp_conversor);

    if (comb == "0") {
      comb = "Reserva";
    } else {
      comb = "Cheio";
    }

    server.handleClient();
  }

  rpm = 0;
  velo = 0;
  cvt_temp = 0;
  comb = "";

  delay(10);
}
