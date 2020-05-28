#include <HCSR04.h>
#include <math.h>
#include <Bridge.h>
#include <BridgeServer.h>
#include <BridgeClient.h>

BridgeServer server;

const int TRIGGER_PIN = 13;
const int ECHO_PIN = 12;
UltraSonicDistanceSensor distanceSensor(TRIGGER_PIN, ECHO_PIN);  // Initialize sensor that uses digital pins TRIGGER and ECHO.

const int DATA_LENGTH = 20;
double data[DATA_LENGTH];

const int BUFF_LENGTH = 5;
int BUFF_INDEX = 0;
double buff[BUFF_LENGTH];
const double SEG1[] = {10.0, 55.0};
const double SEG2[] = {65.0, 105.0};
const double SEG3[] = {115.0, 160.0};


void setup () {
  Serial.begin(9600);  // We initialize serial connection so that we could print values from sensor.
  Bridge.begin();
  server.begin();
  pinMode(12, INPUT);
  pinMode(13, OUTPUT);

  BridgeClient client = server.accept();
  client.println("Status: 200");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Access-Control-Allow-Methods: GET");
  client.println("Content-Type: text/html");
  client.println("Connexion: close");
  client.println();
  client.println("NOPE");
}

double mean(double tab[]) {
  // Calcule la moyenne des valeurs de tab, avec tab de taille DATA_LENGTH
  double res = 0;
  int sum_length = 0;
  for (int i = 0; i < DATA_LENGTH; i++) {
    if (tab[i] != -1.0) {
      res += tab[i];
      sum_length++;
    }
  }
  if (sum_length != 0) {
    res = res / sum_length;
  } else {
    res = -1.0;
  }
  return res;
}

int segment(double buff[]) {
  // Si au moins 60% des valeurs de buff sont dans un domaine de segmentation, renvoie le numéro associé au domaine. Sinon renvoie 0
  // On prendra buff de longueur BUFF_LENGTH
  int min = floor(0.6 * BUFF_LENGTH);
  int in1 = 0;
  int in2 = 0;
  int in3 = 0;
  int res = 0;
  for (int i = 0; i < BUFF_LENGTH; i++) {
    if (buff[i] > SEG1[0] && buff[i] < SEG1[1]) {
      in1++;
    } else if (buff[i] > SEG2[0] && buff[i] < SEG2[1]) {
      in2++;
    } else if (buff[i] > SEG3[0] && buff[i] < SEG3[1]) {
      in3++;
    }
  }
  if (in1 >= min) {
    res = 1;
  } else if (in2 >= min) {
    res = 2;
  } else if (in3 >= min) {
    res = 3;
  }
  return res;
}

void processDistance(BridgeClient client, int segment) {

  if (client) {
    String command = client.readString();
    command.trim();

    if (command == "king") {
      if (segment == 1) {
        client.print("Yes");
      }
      else {
        client.print("No");
      }
    }
    else {
      if (command == "super") {
        if (segment == 1) {
          client.print("<img src=\"chips.jpg\" alt=\"Image Chips\">");
        }
        else {
          client.print("<img src=\"soda.jpg\" alt=\"Image Soda\">");
        }
      }
    }
    client.stop();
  }
}

void loop() {
  // Every 500 miliseconds, do a measurement using the sensor and print the distance in centimeters.
  //Serial.println(distanceSensor.measureDistanceCm());
  if (BUFF_INDEX == BUFF_LENGTH) {
    BUFF_INDEX = 0;
  }
  for (int i = 0; i < DATA_LENGTH; i++) {
    double data_in = distanceSensor.measureDistanceCm();
    //    while (data_in < 0) {
    //      data_in = distanceSensor.measureDistanceCm();
    //    }
    data[i] = data_in;
  }
  buff[BUFF_INDEX] = mean(data);
  BUFF_INDEX++;
  //Serial.println(mean(data));
  Serial.println(segment(buff));

  BridgeClient client = server.accept();
  if (client) {
    String command = client.readString();
    command.trim();

    client.println("Status: 200");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Access-Control-Allow-Methods: GET");
    client.println("Content-Type: text/html");
    client.println("Connexion: close");
    client.println();

    if (command == "king") {
      if (segment(buff)) {
        client.print("Yes");
      }
      else {
        client.print("No");
      }
    }
    else {
      if (command == "super") {
        int val = digitalRead(11);
        if (val) {
          client.print("<img src=\"chips.jpg\" alt=\"Image Chips\">");
        }
        else {
          client.print("<img src=\"soda.jpg\" alt=\"Image Chips\">");
        }
      }
    }
    client.stop();
  }
  delay(200);
}
