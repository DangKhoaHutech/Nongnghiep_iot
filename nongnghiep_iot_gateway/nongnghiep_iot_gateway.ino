#include "certs.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"

// thu vien lora
#include <SoftwareSerial.h>
#include <SPI.h> 
#include <SimpleTimer.h>
SimpleTimer timer;
/*khai bao 2 chân RX và TX*/
SoftwareSerial mySerial(22, 23); 
/*Khai báo chân số 2 là chân led*/
int led = 2;

// Thông tin đăng nhập Wifi
const char *WIFI_SSID = "Tang 1";
const char *WIFI_PASSWORD = "supinnguyen";

// Tên của thiết bị. Điều này phải khớp với tên được xác định trong AWS iot
#define DEVICE_NAME "Nongnghiep_IoT"

// endpoint MQTTT cho thiết bị (duy nhất cho mỗi tài khoản AWS nhưng được chia sẻ giữa các thiết bị trong tài khoản)
#define AWS_IOT_ENDPOINT "aisf7gfxklhgo-ats.iot.us-west-2.amazonaws.com"

// chủ đề MQTT
#define AWS_IOT_TOPIC "$aws/things/"DEVICE_NAME"/shadow/update"

// Số lần kết nối với AWS
#define AWS_MAX_RECONNECT_TRIES 50

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);

void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // thử 15 lần kết nối
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 15){
    delay(500);
    Serial.print(".");
    retries++;
  }

  // nếu kết nối đến wifi không được thì nghỉ 1 phút
  if(WiFi.status() != WL_CONNECTED){
    esp_sleep_enable_timer_wakeup(1 * 60L * 1000000L);
    esp_deep_sleep_start();
  }
}

void connectToAWS()
{
// Định cấu hình WiFiClientSecure để sử dụng chứng chỉ AWS iot
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  //kết nối đến mqtt
  client.begin(AWS_IOT_ENDPOINT, 8883, net);

  // đếm xem có bao nhiêu lần kết nối đến mqtt
  int retries = 0;
  Serial.print("Đang kết nối đến AWS IOT..");

  while (!client.connect(DEVICE_NAME) && retries < AWS_MAX_RECONNECT_TRIES) {
    Serial.print(".");
    delay(100);
    retries++;
  }

  // đảm bảo kết nối thành công đến mqtt
  // nếu không. chúng ta chỉ kết thức và đợi vòng lặp tiếp theo
  if(!client.connected()){
    Serial.println(" Timeout!");
    return;
  }

  // kết nối thành công đến AWS
  // có thể đăng kí chủ đề
  Serial.println("Kết nối thành công !");
}

void sendJsonToAWS(String mess)
{
   /*In thứ tự thứ 12 trong chuỗi 
    nếu là 0 thì mưa, là 1 thì không mưa, là 2 thì không nhận được*/
    String S_rain=mess.substring(12);
    // nhận khí gas
    String S_KhiGas=mess.substring(14);
    /*In thứ tự từ vị trí 0 đến vị trí 4 chỉ số nhiệt độ lấy từ chuỗi*/
    String S_nhietdo=mess.substring(0,4);
    /*In thứ tự từ vị trí 6 đến vị trí 10 chỉ số độ ẩm lấy từ chuỗi*/
    String S_doam=mess.substring(6,10);
    /*Chuyển từ kiểu String sang kiểu int*/
    int rain = S_rain.toInt();
    /*Chuyển từ kiểu String sang kiểu int*/
    int khigas = S_KhiGas.toInt();
    /*Chuyển từ kiểu String sang kiểu float*/
    float nhietdo= S_nhietdo.toFloat();
    float doam= S_doam.toFloat();

  
  StaticJsonDocument<128> jsonDoc;
  JsonObject stateObj = jsonDoc.createNestedObject("state");
  JsonObject reportedObj = stateObj.createNestedObject("reported");
  
  // nhiệt độ, độ ẩm, cường độ wifi
  reportedObj["nhietdo"] = nhietdo;
  reportedObj["doam"] = doam;
  reportedObj["cuongdo_wifi"] = WiFi.RSSI();
  
  // tạo đối tượng lồng 
  JsonObject locationObj = reportedObj.createNestedObject("location");
  locationObj["name"] = "Nongnghiep";

  Serial.println("Gửi thông tin đến AWS...");
  //serializeJson(doc, Serial);
  char jsonBuffer[512];
  serializeJson(jsonDoc, jsonBuffer);

  client.publish(AWS_IOT_TOPIC, jsonBuffer);
}

void nhanGoitin(){
   String message="00.00-00.00-2-2";
    Serial.println(message);
   /*nếu trả về kí tự lớn hơn 1*/
  if(mySerial.available() > 1)
  {
    /*Nhận gói tin*/
    message = mySerial.readString(); 
    Serial.printf("Gateway nhận dữ liệu từ Sensor node:\n");
          /*In chuỗi ra màng hình*/
    Serial.println(message);
    /*Đèn LED sáng khi nhận được dữ liệu*/
    digitalWrite(led,HIGH);
    delay(200);/*dừng 0.2 giây sau đó tắt*/
    digitalWrite(led,LOW);
    sendJsonToAWS(message);
  }

}
void setup() {
    pinMode(led,OUTPUT);/*mở chân led*/
    delay(1000);
    Serial.begin(9600);
    mySerial.begin(9600);
  connectToWiFi();
  connectToAWS();
}

void loop() {
    
  nhanGoitin();
  client.loop();
  delay(4000);
}
