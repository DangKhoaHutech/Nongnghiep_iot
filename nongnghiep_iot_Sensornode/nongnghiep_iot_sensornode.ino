//Woon Jun Shen
//UM402 (433 MHz UART)
#include <SoftwareSerial.h>// Thư viện Lora.
#include<DHT.h>// Thư viện cảm biến nhiệt độ, độ ẩm.
#include <SPI.h>


#define DHTPIN 21// setup chân 21 cho cảm biến.
#define RAINPIN 18 // setup chan 18 cho cảm biến mưa.
#define DHTTYPE DHT11 // Khai báo loại cảm biến.
DHT dht (DHTPIN,DHTTYPE);
int led = 2;
int KhiGasA0 = 15;// Chân 15 cảm biến khí gas
int sensorThres = 300; // ngưỡng giới hạn của khí gas
int value2=0;

int freq = 2000;
int channel = 0;
int resolution = 8;
int dutyCycle = 128;
const int buzzer =  4;

SoftwareSerial mySerial(22,23); //chân TX, chân RX (truyền và nhận tín hiệu)
void setup() { 
  Serial.begin(9600); //đặt tốc độ baud là 9600
  pinMode(RAINPIN,INPUT); //Đặt chân 19 là chân input
  pinMode(KhiGasA0, INPUT);// mở Chân cảm biến khí Gas
  pinMode(buzzer, INPUT);
  mySerial.begin(9600);  
  dht.begin();
}

void loop() {
      
    // code cảm biến nhiệt độ độ ẩm từ cảm biến DHT11
      float nhietdo= dht.readTemperature(); // lấy giá trị của nhiệt độ từ sensor.
      float doam= dht.readHumidity();// lấy giá trị của độ ẩm từ sensor.
      if(isnan(nhietdo) ||isnan(doam)){
      Serial.println("Lỗi từ cảm biến !");
      return;
    }
    // chuyển đổi nhiệt độ và độ ẩm thành kiểu chuỗi
    String S_nhietdo=String(nhietdo).c_str(); 
    String S_doam= String(doam).c_str();
    
    Serial.printf("Cách 5 giây dữ liệu sẽ gửi đến GateWay 1 lần:\n*nhiệt độ: %s ; độ ẩm: %s \n",String(nhietdo).c_str(),String(doam).c_str());

//---------------------------------------------------------------------------
    int analogSensor = analogRead(KhiGasA0);// lấy giá trị của cảm biến khí gas
      Serial.print("Pin A0: ");
      Serial.println(analogSensor);
    
     if (analogSensor > sensorThres) // nếu giá trị vượt ngưỡng thì thông báo
    {
      Serial.println("Phát hiện khí Gas. :("); value2=1;
      Serial.println("Phát cảnh báo");
         pinMode(buzzer, OUTPUT); 
    }
    else
    {
      Serial.println("Không phát hiện khí Gas :))) !"); value2=0;
      Serial.println("Tắt cảnh báo");
       pinMode(buzzer, INPUT); 
    }
//-------------------------------------------------------------------------
  //code cảm biến mưa từ cảm biến V2.
    int value = digitalRead(RAINPIN);// đọc tín hiệu từ cảm biến v2
    if(value == 0)// nếu giá trị == high =(1) tức là không mưa và ngược lại
    {Serial.println("không mưa");}
    else
    {Serial.println("có mưa");}

//------------------------------------------------------------------------------
    Serial.printf("chuỗi dữ liệu gửi đi: %0.2f-%0.2f-%d-%d\n",nhietdo,doam,value,value2);
    delay(5000);// Dừng 5 giây cho 1 lần gửi.
 
 //----------------------------------------------------------------------------
    // gửi dữ liệu gửi qua getway.
      mySerial.printf("%0.2f-%0.2f-%d-%d",nhietdo,doam,value);// nối chuỗi và gửi tín hiệu về getway lora.
      digitalWrite(led,HIGH); // đèn led sáng khi Gửi được tín hiệu.
      delay(500);
      digitalWrite(led,LOW);

}
