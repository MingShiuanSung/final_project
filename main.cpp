#include "mbed.h"
#include "bbcar.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"

// WIFI
WiFiInterface *wifi;
volatile int message_num = 0;
void messageArrived(MQTT::MessageData&);
void publish_message(char*);
MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);

Ticker servo_ticker;
Ticker encoder_ticker0;
Ticker encoder_ticker1;
Timer t; 

PwmOut pin8(D8), pin9(D9);  // pin8 left wheel; pin9 right wheel
Serial pc(USBTX,USBRX); //tx,rx
Serial uart(D12,D11); //tx,rx
DigitalInOut ping(D10);
DigitalIn pin3(D3);  // left wheel
DigitalIn pin4(D4);  // left wheel
BBCar car(pin8, pin9, servo_ticker);

Thread thread1;
Thread thread2;
void recieve_thread(void);
void send_thread(void);
bool flag = false;

void ping(void);

int main() {

    uart.baud(9600);
    thread1.start(send_thread);
    thread2.start(recieve_thread);

    parallax_encoder encoder0(pin3, encoder_ticker0);
    parallax_encoder encoder1(pin4, encoder_ticker1);
    encoder0.reset();
    encoder1.reset();

    //---configure WIFI settings---
   wifi = WiFiInterface::get_default_instance();
   if (!wifi) {
      printf("ERROR: No WiFiInterface found.\r\n");
      return -1;
   }

   printf("\nConnecting to %s...\r\n", MBED_CONF_APP_WIFI_SSID);
   int ret = wifi->connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
   if (ret != 0) {
      printf("\nConnection error: %d\r\n", ret);
      return -1;
   }
   
   NetworkInterface* net = wifi;
   MQTTNetwork mqttNetwork(net);

   const char* host = "192.168.0.103";
   const char* topic = "Mbed";
   printf("Connecting to TCP network...\r\n");
   int rc = mqttNetwork.connect(host, 1883);
   if (rc != 0) {
      printf("Connection error.");
      return -1;
   }
   printf("Successfully connected!\r\n");

   MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
   data.MQTTVersion = 3;
   data.clientID.cstring = "Mbed";

   if ((rc = client.connect(data)) != 0){
      printf("Fail to connect MQTT\r\n");
   }
   if (client.subscribe(topic, MQTT::QOS0, messageArrived) != 0){
      printf("Fail to subscribe\r\n");
   }

    //---starting the mission---
    //---enter track1---
    publish_message("going straight for 125cm.");
    car.goStraight(100); // go straight at speed 100
    encoder0.reset();
    while(encoder0.get_cm() < 125) wait_ms(50);  // go straight 125cm
    car.stop();

    publish_message("turn left 90 degree.");
    car.turn(100, 0.1);  // turn left 90 degree
    encoder1.reset();
    while(encoder1.get_cm() < 21) wait_ms(50);
    car.stop();

    //---enter the mission1---
    publish_message("enter mission1.")
    publish_message("go straight 90cm.");
    car.goStraight(100); // go straight at speed 100
    encoder0.reset();
    while(encoder0.get_cm() < 90) wait_ms(50);  // go straight 90cm
    car.stop(); 

    publish_message("reverse parking.");
    car.turn(-100, 0.1);  // turn backward right 90 degree
    encoder1.reset();
    while(encoder1.get_cm() < 10) wait_ms(50);  
    car.stop();

    publish_message("go backward 30cm.")
    car.goStraight(-100); // go backward at speed 100
    encoder0.reset();
    while(encoder0.get_cm() < 30) wait_ms(50);  // go backward 30cm
    car.stop(); 

    publish_message("go forward 30cm");
    car.goStraight(100); // go forward at speed 100
    encoder0.reset();
    while(encoder0.get_cm() < 30) wait_ms(50);  // go forward 30cm
    car.stop(); 

    publish_message("turn right 90 degree.");
    car.turn(100, -0.1);  // turn right 90 degree
    encoder0.reset();
    while(encoder0.get_cm() < 10) wait_ms(50);  
    car.stop();

    publish_message("go straight 40cm.");
    car.goStraight(100); // go straight at speed 100
    encoder0.reset();
    while(encoder0.get_cm() < 40) wait_ms(50);  // go straight 40cm
    car.stop(); 

    publish_message("reverse parking.");
    car.turn(-100, -0.1);  // turn backward left 90 degree
    encoder1.reset();
    while(encoder0.get_cm() < 10) wait_ms(50);  
    car.stop();

    publish_message("go backward 30cm.");
    car.goStraight(-100); // go backward at speed 100
    encoder0.reset();
    while(encoder0.get_cm() < 30) wait_ms(50);  // go backward 30cm
    car.stop(); 

    //---take picture of MNIST (triggered by setting flag to true)---
    publish_message("taking picture of MNIST foto.")
    flag = true;
    wait(0.5);
    flag = false;
    wait(2.0);

    publish_message("go straight 30cm.");
    car.goStraight(100); // go straight at speed 100
    encoder0.reset();
    while(encoder0.get_cm() < 30) wait_ms(50);  // go straight 30cm
    car.stop(); 

    publish_message("turn right 90 degree.");
    car.turn(100, -0.1);  // turn right 90 degree
    encoder0.reset();
    while(encoder0.get_cm() < 21) wait_ms(50);  
    car.stop();

    publish_message("go straight 5cm.");
    car.goStraight(100); // go straight at speed 100
    encoder0.reset();
    while(encoder0.get_cm() < 5) wait_ms(50);  // go straight 5cm
    car.stop(); 

    publish_message("turn right 90 degree.");
    car.turn(100, -0.1);  // turn right 90 degree
    encoder0.reset();
    while(encoder0.get_cm() < 21) wait_ms(50);  
    car.stop();

    //---enter track2---
    publish_message("heading to mission2.");
    publish_message("go straight 90cm.");
    car.goStraight(100); // go straight at speed 100
    encoder0.reset();
    while(encoder0.get_cm() < 100) wait_ms(50);  // go straight 90cm
    car.stop(); 

    publish_message("turn right 90 degree.")
    car.turn(100, -0.1);  // turn right 90 degree
    encoder0.reset();
    while(encoder0.get_cm() < 22) wait_ms(50);  
    car.stop();

    //---enter mission2---
    publish_message("go straight 5cm.");
    car.goStraight(100); // go straight at speed 100
    encoder0.reset();
    while(encoder0.get_cm() < 5) wait_ms(50);  // go straight 5cm
    car.stop(); 

    publish_message("turn right 90 degree.");
    car.turn(100, -0.1);  // turn right 90 degree
    encoder0.reset();
    while(encoder0.get_cm() < 22) wait_ms(50);  
    car.stop();

    publish_message("turn left 90 degree.");
    car.turn(100, 0.1);  // turn left 90 degree
    encoder1.reset();
    while(encoder1.get_cm() < 22) wait_ms(50);  
    car.stop();

    publish_message("go straight 40cm slowly.")
    car.goStraight(25); // go straight at speed 25
    encoder0.reset();
    while(encoder0.get_cm() < 25) wait_ms(50);  // go straight 40cm
    car.stop();

    //---ping detect shape---
    publish_message("scanning the shape.");
    ping();
    //---ping finish detect shape---

    publish_message("turn left 90 degree.");
    car.turn(100, 0.1);  // turn left 90 degree
    encoder1.reset();
    while(encoder1.get_cm() < 22) wait_ms(50);  
    car.stop();

    publish_message("turn right 90 degree.");
    car.turn(100, -0.1);  // turn right 90 degree
    encoder0.reset();
    while(encoder0.get_cm() < 22) wait_ms(50);  
    car.stop();

    publish_message("go straight 25cm.")
    car.goStraight(100); // go straight at speed 100
    encoder0.reset();
    while(encoder0.get_cm() < 25) wait_ms(50);  // go straight 25cm
    car.stop(); 

    //---exit the map---
    publish_message("heading toward exit.");
    publish_message("turn right 90 degree.");
    car.turn(100, -0.1);  // turn right 90 degree
    encoder0.reset();
    while(encoder0.get_cm() < 22) wait_ms(50);  
    car.stop();

    publish_message("go straight 100cm.");
    car.goStraight(100); // go straight at speed 100
    encoder0.reset();
    while(encoder0.get_cm() < 100) wait_ms(50);  // go straight 100cm
    car.stop(); 
    publish_message("end of the mission.");

}

void recieve_thread(){

   while(1) {

      if(uart.readable()){

            char recv = uart.getc();

            pc.putc(recv);

            pc.printf("\r\n");

      }

   }

}


void send_thread(){

   while(1){

      if(flag == true){

         char s[21];

         sprintf(s,"image_classification");

         uart.puts(s);

         pc.printf("send\r\n");

         wait(0.5);

      }

   }

}

void ping()
{
   float val, val_prev, val_prev2;  // val is the current time interval; 
                                    // val_prev is the previous time interval
                                    // val_prev2 is previous to previous time interval

   bool flag_inc = false, flag_dec = false; // flag_inc detect the increase of ping
                                            // flag_dec detect the decrease of ping

   bool inc_first = false, dec_first = false; // inc_first detect increase prior to decrease
                                              // dec_first detect decrease prior to increase                           

   car.turn(100, 0.1);  // turn left 45 degree
   encoder1.reset();
   while(encoder1.get_cm() < 10) wait_ms(50);  
   car.stop();
   car.turn(100, -0.1);  // turn right 45 degree
   encoder0.reset();

   while(encoder0.get_cm() < 22) {

      ping.output();
      ping = 0; wait_us(200);
      ping = 1; wait_us(5);
      ping = 0; wait_us(5);
 
      ping.input();
      while(ping.read()==0);
      t.start();
      while(ping.read()==1);
      val = t.read();
      if (val_prev2 > val_prev && val_prev > val) flag_inc = true;
      if (val_prev2 < val_prev && val_prev < val) flag_dec = true;
      if (flag_inc == true && flag_dec != true) inc_first = true;
      if (flag_inc != true && flag_dec == true) dec_first = true;
      val_prev2 = val_prev;
      val_prev = val;

      t.stop();
      t.reset();
      wait_ms(50);
   }

   if (inc_first == true && flag_dec == true) // number 3
   {
      publish_message("V-shape detected!");
   }
   else if (inc_first == true && flag_dec != true) // number 2
   {
      publish_message("right triangle detected!");
   }
   else if (flag_inc == false && flag_dec == false) // number 1
   {
      publish_message("square detected!");
   }
   else if (dec_first == true && flag_inc == true) // number 0
   {
      publish_message("Equilateral triangle detected!");
   }
   else // false detection
   {
      publish_message("nothing detected!");
   }
   
   car.turn(100, 0.1);  // turn left 45 degree
   encoder1.reset();
   while(encoder1.get_cm() < 10) wait_ms(50);
   car.stop();
}

void messageArrived(MQTT::MessageData& md) {
   MQTT::Message &message = md.message;
   char msg[300];
   sprintf(msg, "Message arrived: QoS%d, retained %d, dup %d, packetID %d\r\n", message.qos, message.retained, message.dup, message.id);
   printf(msg);
   wait_ms(1000);
   char payload[300];
   sprintf(payload, "Payload %.*s\r\n", message.payloadlen, (char*)message.payload);
   printf(payload);
}

void publish_message(char* buff)
{
   MQTT::Message message;

   message.qos = MQTT::QOS0;
   message.retained = false;
   message.dup = false;
   message.payload = (void*) buff;
   message.payloadlen = strlen(buff) + 1;
   rc = client.publish(topic, message);

   printf("rc:  %d\r\n", rc);
   printf("Puslish message: %s\r\n", buff);
   client.yield(10);
}