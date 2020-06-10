#include <HCSR04.h>
#include <math.h>
#include <Bridge.h>
#include <BridgeServer.h>
#include <BridgeClient.h>

BridgeServer server;

const int TRIGGER_PIN = 13;
const int ECHO_PIN = 12;
UltraSonicDistanceSensor distanceSensor(TRIGGER_PIN, ECHO_PIN);  // Initialize sensor that uses digital pins TRIGGER and ECHO.

const int DATA_LENGTH = 10; // 20
double data[DATA_LENGTH];

const int BUFF_LENGTH = 5;
int BUFF_INDEX = 0;
double buff[BUFF_LENGTH];
const double SEG1[] = {10.0, 55.0};
const double SEG2[] = {65.0, 105.0};
const double SEG3[] = {115.0, 160.0};

String command;

void setup () {
  Serial.begin(9600);  // We initialize serial connection so that we could print values from sensor.
  Bridge.begin();
  //server.listenOnLocalhost();
  server.begin();
  
  pinMode(12, INPUT);
  pinMode(13, OUTPUT);

  pinMode(11, INPUT);
  pinMode(9, INPUT);
  
  if (digitalRead(9)*digitalRead(11) == 1) {
    command = "soggy";
  }
  else {
    if (digitalRead(9) == 1) {
      command = "king";
    }
    else {
      if (digitalRead(11) == 1) {
        command = "super";
      }
      else {
        command = "none";
      }
    }
  }
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

void loop() {
  // Every 500 miliseconds, do a measurement using the sensor and print the distance in centimeters.
  //Serial.println(distanceSensor.measureDistanceCm());
  if (BUFF_INDEX == BUFF_LENGTH) {
    BUFF_INDEX = 0;
  }
  for (int i = 0; i < DATA_LENGTH; i++) {
    Serial.println(i);
    double data_in = distanceSensor.measureDistanceCm();
    while (data_in < 0) {
      data_in = distanceSensor.measureDistanceCm();
    }
    data[i] = data_in;
  }
  buff[BUFF_INDEX] = mean(data);
  BUFF_INDEX++;
  Serial.println(mean(data));
  // Serial.println(segment(buff));

  BridgeClient client = server.accept();
  
  if (client) {
//    command = client.readString();
//    command.trim();
    
    client.println("Status: 200");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Access-Control-Allow-Methods: GET");
    client.println("Content-Type: text/html");
    client.println("Connexion: close");
    client.println();

    int seg = segment(buff);
    if (command == "king") {
      if (seg == 1) {
        client.print("<img class=\"product_pict\" src=\"plate.png\" alt=\"Image Assiette\">");
      }
      else {
        if (seg == 2) {
          client.print("<img class=\"product_pict\" src=\"frites.png\" alt=\"Image Frites\">");
        }
        else {
          if (seg == 3) {
            client.print("<img class=\"product_pict\" src=\"kebab.png\" alt=\"Image Kebab\">");
          }
          else {
            client.print("<h3>Personne n'est détecté devant ce capteur, veuillez vérifier votre position !</h3>");
          }
        }
      }
    }
    else {
      if (command == "super") {
        if (seg == 1) {
          client.print("<img class=\"product_pict\" src=\"snack.png\" alt=\"Image Chips\">");
        }
        else {
          if (seg == 2) {
            client.print("<img class=\"product_pict\" src=\"pizza.png\" alt=\"Image Pizza\">");
          }
          else {
            if (seg == 3) {
              client.print("<img class=\"product_pict\" src=\"can.png\" alt=\"Image Canette\">");
            }
            else {
              client.print("<h3>Personne n'est détecté devant ce capteur, veuillez vérifier votre position !</h3>");
            }
          }
        }
      }
      else {
        if (command == "soggy") {
          if (seg == 1) {
            client.print("<img class=\"product_pict\" src=\"soda.png\" alt=\"Image Soda\">");
          }
          else {
            if (seg == 2) {
              client.print("<img class=\"product_pict\" src=\"pinte.png\" alt=\"Image Pinte\">");
            }
            else {
              if (seg == 3) {
                client.print("<img class=\"product_pict\" src=\"saucisson.png\" alt=\"Image Saucisson\">");
              }
              else {
                client.print("<h3>Personne n'est détecté devant ce capteur, veuillez vérifier votre position !</h3>");
              }
            }
          }
        }
        else {
          client.print("<h3>Veuillez vérifier les branchements</h3>");
        }
      }
    }
    client.stop();
  }
}
