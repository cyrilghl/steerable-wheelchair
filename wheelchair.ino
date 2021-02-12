#include <LiquidCrystal.h>
#include <Servo.h>;
#include <IRremote.h>;

//Servo
Servo leftServo;
Servo rightServo;

//plages de consignes du servos
const int fastFwd = 135;
const int slowFwd = 95;
const int slowBwd = 85;
const int fastBwd = 45;

//CONST IR
const long MINUS = 0xFD906F;
const long PLUS = 0xFD807F;
const long ON_OFF = 0xFD00FF;
const long ZERO = 0xFD30CF;
const long ONE = 0xFD08F7;
const long TWO = 0xFD8877;
const long THREE = 0xFD48B7;
const long FOUR = 0xFD28D7;
const long FIVE = 0xFDA857;
const long SIX = 0xFD6897;
const long SEVEN = 0xFD18E7;
const long EIGHT = 0xFD9867;
const long NINE = 0xFD58A7;

//Déclaration des constantes de branchements
const int leftServoPin = 11; // Utilisation de branche pwm
const int rightServoPin = 10;
const int fwdUS = A0;
const int bwdUS = A1; //backward Ultrasonic Sensor
const int buzzerPin = 1;
const int bwdLed2 = 12; //Led arriere
const int bwdLed1 = 13;
const int fwdLed2 = 8; // led avant
const int fwdLed1 = 9;
const int presencePin = A2; //capt lumiere
const int recPin = A3;      // pin recepteur IR
const int xAxis = A4;       // potentiometre x
const int yAxis = A5;       // potentiometre y

//IR
IRrecv irrecv(recPin);
decode_results ir;
long irValue = 0;      // valeur recue par le recepteur ir
int presenceValue = 0; // valeur lumineuse
int cm = 0;
int distanceFwd = 50; // distance avant
int distanceBwd = 50; // distance arriere
//Variables des potentiometres
int xPot;
int yPot;
//Servos
int rightSpeed = 90; // consignes de vitesse
int leftSpeed = 90;
int leftServoPosition = 90; // position angulaire du servo
int rightServoPosition = 90;
int choice = 0; // 0 = menu 1= joystick 2 = ir
//
bool someoneOnChair = false;

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);
String line0;
String line1;
String tempLine0;
String tempLine1;
String usValue;

void setup()
{
    // set up the LCD's number of columns and rows:
    lcd.begin(16, 2);
    pinMode(fwdUS, INPUT);
    pinMode(bwdUS, INPUT);
    pinMode(presencePin, INPUT);
    pinMode(buzzerPin, OUTPUT);
    pinMode(fwdLed2, OUTPUT);
    pinMode(fwdLed1, OUTPUT);
    pinMode(bwdLed2, OUTPUT);
    pinMode(bwdLed1, OUTPUT);
    leftServo.attach(leftServoPin);
    rightServo.attach(rightServoPin);
    leftServo.write(90);
    rightServo.write(90);
    irrecv.enableIRIn();
    testComponents();
    if (someoneOnChair == true)
    {
        tempLine0 = "JOYSTICK";
        tempLine1 = "";
    }
    else
    {
        tempLine0 = "IR";
        tempLine1 = "";
    }
}

void loop()
{

    readPotentiometerValue();
    distanceFwd = readUltrasonicDistance(fwdUS);
    distanceBwd = readUltrasonicDistance(bwdUS);

    leftServoPosition = leftServo.read();
    rightServoPosition = rightServo.read();
    someoneOnChair = isSomeoneOnChair();
    //On suit les instructions de l'utilisateur
    choiceControlMode();

    if (choice == 1 && someoneOnChair)
    {
        joystickControl();
        rampControl();
    }
    else if (choice == 2)
    {
        remoteControl();
        rampControl();
    }
    usValue = String(distanceFwd);

    if (line0 != tempLine0 || line1 != tempLine1)
    {
        if (line0 != tempLine0)
        {
            line0 = tempLine0;
            lcd.setCursor(0, 0);
            lcd.print("               ");
            lcd.setCursor(0, 0);
            lcd.print(line0);
        }

        if (line1 != tempLine1)
        {
            line1 = tempLine1;
            lcd.setCursor(0, 1);
            lcd.print("                  ");
            lcd.setCursor(0, 1);
            lcd.print(line1);
        }

        delay(50);
    }
}

void readPotentiometerValue()
{
    //Premiere étape : on lit les valeurs des différents potentiometres.
    xPot = analogRead(xAxis);
    yPot = analogRead(yAxis);

    //On les réduit a une plage de valeur [0;180]
    //En effet les valeurs du potentiometres sont comprises entre 0 et 1023
    xPot = map(xPot, 0, 1023, 0, 180);
    yPot = map(yPot, 0, 1023, 0, 180);
}
//Lire valeur sur lcd
long readUltrasonicDistance(int pin)
{
    pinMode(pin, OUTPUT); // Clear the trigger
    digitalWrite(pin, LOW);
    delayMicroseconds(2);
    // Sets the pin on HIGH state for 10 micro seconds
    digitalWrite(pin, HIGH);
    delayMicroseconds(10);
    digitalWrite(pin, LOW);
    pinMode(pin, INPUT);
    // Reads the pin, and returns the sound wave travel time in microseconds
    return 0.01723 * pulseIn(pin, HIGH);
}

// METHODE MENU CHOIX DU GUIDAGE
void choiceControlMode()
{
    getIrValue();
    if (choice == 0 || irValue == ZERO)
    {
        leftServo.detach();
        rightServo.detach();
        tempLine0 = "** MENU **";
        tempLine1 = "1: Joy | 2: IR";
        if (irValue == ZERO)
        {
            choice = 0;
        }
        else if (irValue == ONE)
        {
            tempLine0 = "Joystick";
            tempLine1 = "Mode";
            choice = 1;
        }
        else if (irValue == TWO)
        {
            tempLine0 = "IR";
            tempLine1 = "Mode";
            choice = 2;
            irValue = FIVE;
        }
        else
        {
            choice = 0;
        }
    }
}
// METHODE RECUPERER VALEUR IR
void getIrValue()
{
    if (irrecv.decode(&ir))
    {
        irValue = ir.value;
        Serial.println(irValue, HEX);
        irrecv.resume();
    }
}

//Ecire un msg sur le LCD
void displayMsg(int line, String msg)
{
    lcd.setCursor(0, line);
    lcd.print(msg);
    lcd.setCursor(10, line);
    delay(100);
}

//Verifier si quelqu'un est présent sur la chaise
bool isSomeoneOnChair()
{
    presenceValue = analogRead(presencePin);
    if (presenceValue < 400)
    {
        return true;
    }
    else
    {
        return false;
    }
}
//tester la quasi totalité des composants
void testComponents()
{
    delay(50);
    ///////////////lcd.clear();
    if (isSomeoneOnChair())
    {
    }
    else
    {
    }
    delay(50);
    //////////////lcd.clear();

    turnOnLed();
    //////////////lcd.clear();

    leftServo.write(0);
    rightServo.write(180);
    delay(50);
    leftServo.write(90);
    rightServo.write(90);
    //////////////lcd.clear();

    digitalWrite(buzzerPin, HIGH);
    //tone(13,200,2000);
    delay(50);
    digitalWrite(buzzerPin, LOW);
    //////////////lcd.clear();
    delay(50);
    turnOffLed();
    //////////////lcd.clear();
}

// METHODE D'ALERTE SONORE ET VISUELLE
void warningAlert()
{
    for (int i = 0; i < 3; i++)
    {
        digitalWrite(buzzerPin, HIGH);
        delay(100);
        digitalWrite(buzzerPin, LOW);
        delay(100);
    }
  tempLine1="Obstacle proche";
}

void turnOffLed()
{
    digitalWrite(bwdLed1, LOW); //led1 eteinte
    digitalWrite(bwdLed2, LOW); //led2 eteinte
    digitalWrite(fwdLed1, LOW); //led1 eteinte
    digitalWrite(fwdLed2, LOW); //led2 eteinte
}
void turnOnLed()
{
    digitalWrite(bwdLed1, HIGH); //led1 eteinte
    digitalWrite(bwdLed2, HIGH); //led2 eteinte
    digitalWrite(fwdLed1, HIGH); //led1 eteinte
    digitalWrite(fwdLed2, HIGH); //led2 eteinte
}

void turnOnFwdLed()
{
    digitalWrite(fwdLed1, HIGH); //led1 eteinte
    digitalWrite(fwdLed2, HIGH); //led2 eteinte
}

void turnOnBwdLed()
{
    digitalWrite(bwdLed1, HIGH); //led1 eteinte
    digitalWrite(bwdLed2, HIGH); //led2 eteinte
}

// METHODE RENVOIE VVRAI SI ZONE DE DANGER
bool isVeryUrgent()
{
    if ((distanceFwd <= 10) || (distanceBwd <= 10))
    {
        return true;
    }
    else
    {
        return false;
    }
}

// METHODE RENVOIE VRAIE SI ZONE DE RISQUE
bool isDangerous()
{
    if ((distanceFwd <= 40) || (distanceBwd <= 40))
    {
        return true;
    }
    else
    {
        return false;
    }
}

// METHODE D'APPLICATION DES CONSIGNES PAR RAMPES
void rampControl()
{
    leftServoPosition = leftServoPosition / 5;
    leftServoPosition = leftServoPosition * 5;
    rightServoPosition = rightServoPosition / 5;
    rightServoPosition = rightServoPosition * 5;
    delay(5);
    if (leftServoPosition < leftSpeed)
    {
        leftServo.write(leftServoPosition + 5);
    }
    else if (leftServoPosition > leftSpeed)
    {
        if (leftServoPosition > 90 && leftSpeed < 90)
        {
            leftServo.write(leftServoPosition - 10);
        }
        else
        {
            leftServo.write(leftServoPosition - 5);
        }
    }
    else
    {
    }

    if (rightServoPosition < rightSpeed)
    {
        rightServo.write(leftServoPosition + 5);
    }
    else if (rightServoPosition > rightSpeed)
    {
        if (rightServoPosition > 90 && rightSpeed < 90)
        {
            rightServo.write(rightServoPosition - 20);
        }
        else
        {
            rightServo.write(rightServoPosition - 10);
        }
    }
    else
    {
    }
}

void joystickControl()
{
    if ((xPot >= slowBwd && xPot <= slowFwd) && (yPot >= slowBwd && yPot <= slowFwd) || isVeryUrgent())
    {
         //Alarme
        //////////////lcd.clear();
        tempLine1 = "Arret";
        if (distanceFwd < 10)
        {
            tempLine0 = "Obstacle : " + String(distanceFwd);
          digitalWrite(buzzerPin, HIGH);
        }
        else if (distanceBwd < 10)
        {
            tempLine0 = "Obstacle : " + String(distanceBwd);
          digitalWrite(buzzerPin, HIGH);
        }
        leftServo.write(90);
        rightServo.write(90);
        leftServo.detach(); // Les servomoteurs ne tournent plus
        rightServo.detach();
        turnOffLed();
        //on attend
        delay(50);
    }
    else
    {
        leftServo.attach(leftServoPin);
        rightServo.attach(rightServoPin);
        digitalWrite(buzzerPin, LOW);
        //////////////lcd.clear();
        if ((yPot > slowFwd) && (xPot >= slowBwd && xPot < slowFwd))

        {
            //NB : entre 0 et 90, on recule, entre 90 et 180, on avance.
            if (yPot > fastFwd && distanceFwd > 40)
            //avance rapide possible seulement si l'obstacle est a plus de 40cm
            {
                //On execute la vitesse maximale des deux servos
                leftSpeed = 180;
                rightSpeed = 180;
                //////////////lcd.clear();
                tempLine0 = "Obstacle : " + String(distanceFwd);
                tempLine1 = "Avant rapide";
            }
            else
            {
                //on execute une vitesse moyenne des servos
                rightSpeed = 110;
                leftSpeed = 110;
                //////////////lcd.clear();
                tempLine0 = "Obstacle :" + String(distanceFwd);
                tempLine1 = "Avant lent";
            }
        }
        //RECULER
        else if (yPot < slowBwd && xPot >= slowBwd && xPot < slowFwd)
        {
            //idem
            if (yPot < fastBwd)
            {
                rightSpeed = 0;
                leftSpeed = 0;
                //////////////lcd.clear();
                tempLine0 = "Obstacle : " + String(distanceBwd);
                tempLine1 = "Recul rapide";
            }
            else
            {
                rightSpeed = 75;
                leftSpeed = 75;
                //////////////lcd.clear();
                tempLine0 = "Obstacle : " + String(distanceBwd);
                tempLine1 = "Recul lent";
            }
        }
        else if (xPot < slowBwd && yPot >= slowBwd && yPot <= slowFwd)
        //DROITE
        {
            //On fait bloquer le moteur droit et avancer le moteur gauche de sorte a tourner sur soi même dans le sens horraire
            if (xPot > fastBwd)
            {
                rightSpeed = 90;
                leftSpeed = 120;
                tempLine1 = "Gauche lente";
                tempLine0 = "Obstacle : " + String(distanceFwd);
            }
            else
            {
                rightSpeed = 90;
                leftSpeed = 180;
                tempLine1 = "Gauche rapide";
                tempLine0 = "Obstacle : " + String(distanceFwd);
            }
        }
        //GAUCHE
        //réciproque dans le sens anti-horaire
        else if (xPot > slowFwd && yPot >= slowBwd && yPot <= slowFwd)
        {
            if (xPot < fastFwd)
            {
                rightSpeed = 120;
                leftSpeed = 90;
                tempLine1 = "Droite lente";
                tempLine0 = "Obstacle : " + String(distanceFwd);
            }
            else
            {
                rightSpeed = 180;
                leftSpeed = 90;
                tempLine1 = "Droite rapide";
                tempLine0 = "Obstacle : " + String(distanceFwd);
            }
        }
        //AVANT DROIT
        else
        {
            if ((xPot > slowFwd) && (yPot >= slowFwd))
            {
                //On baisse la vitesse du moteur droit et on fait tourner le gauche dans le sens pour avancer
                rightSpeed = 120;
                leftSpeed = 100;
                tempLine1 = "Avant droit";
                tempLine0 = "Obstacle : " + String(distanceFwd);
            }
            //AVANT GAUCHE
            else if ((xPot < slowBwd) && (yPot >= slowFwd))
            {
                //reciproque
                // rightSpeed = 100;
                //leftSpeed = 120;
                tempLine1 = "Avant gauche";
                tempLine0 = "Obstacle : " + String(distanceFwd);
            }
            // ARRIERE GAUCHE
            else if (xPot < slowBwd && yPot < slowBwd)
            {
                // réciproque dans le sens inverse
                // rightSpeed = 80;
                //leftSpeed = 50;
                tempLine1 = "Arriere gauche";
                tempLine0 = "Obstacle : " + String(distanceBwd);
            }
            //ARRIERE DROIT
            else if (xPot > slowFwd && yPot < slowBwd)
            {
                // rightSpeed = 50;
                //leftSpeed = 80;
                tempLine1 = "Arriere droit";
                tempLine0 = "Obstacle : " + String(distanceBwd);
            }
        }
        if (yPot <= slowBwd)
        {
            if (isDangerous())
            {
                warningAlert();
            }
            turnOffLed();
            turnOnFwdLed();
        }
        else if (yPot >= slowFwd)
        {
            if (isDangerous())
            {
                warningAlert();
            }
            turnOffLed();
            turnOnBwdLed();
        }
    }
}

//Fonction de guidage par télécommande infrarouge
void remoteControl()
{
    getIrValue();
    //CAS ARRET
    if (irValue == FIVE || isVeryUrgent())
    {
        //////////////lcd.clear();
        if (distanceFwd < 10)
        {
            tempLine0 = "Obstacle : " + String(distanceFwd);
          digitalWrite(buzzerPin, HIGH);
        }
        else if (distanceBwd < 10)
        {
            tempLine0 = "Obstacle : " + String(distanceBwd);
          digitalWrite(buzzerPin, HIGH);
        }
        tempLine1 = "Arret";
        rightServo.detach(); // Les servomoteurs ne tournent plus
        leftServo.detach();
    }
    else
    {
        //////////////lcd.clear();
        leftServo.attach(leftServoPin);
        rightServo.attach(rightServoPin);
        // AVANT
        if (irValue == TWO && (distanceFwd > 10))

        {
            //NB : entre 0 et 90, on recule, entre 90 et 180, on avance.
            leftSpeed = 140;
            rightSpeed = 140;
            //////////////lcd.clear();
            tempLine0 = "Obstacle : " + String(distanceFwd);
            tempLine1 = "Avance";
        }
        //RECULER
        else if (irValue == EIGHT && (distanceBwd > 10))
        {
            //idem
            rightSpeed = 40;
            leftSpeed = 40;
            //////////////lcd.clear();
            tempLine0 = "Obstacle : " + String(distanceBwd);
            tempLine1 = "Recul";
        }
        //DROITE
        else if (irValue == SIX)
        {

            rightSpeed = 90;
            leftSpeed = 120;
            tempLine0 = "Obstacle : " + String(distanceFwd);
            tempLine1 = "Droite";
        }
        //GAUCHE
        //réciproque dans le sens anti-horaire
        else if (irValue == FOUR)
        {
            rightSpeed = 120;
            leftSpeed = 90;
            tempLine0 = "Obstacle : " + String(distanceFwd);
            tempLine1 = "Gauche";
        }
    }
  	infraredSpeedControl();
    if (irValue == ZERO)
    {
        choice = 0;
    }
    
    // LES CONSIGNES SONT MAINTENANTS STOCKES
}

void infraredSpeedControl()
{
    if (irValue == PLUS)
    {
        rightSpeed = rightServoPosition + 5;
        leftSpeed = leftServoPosition + 5;
        tempLine0 = "";
        tempLine1 = "Acceleration";
        delay(150);
        irValue = 0;
    }
    else if (irValue == MINUS)
    {
        rightSpeed = rightServoPosition - 5;
        leftSpeed = leftServoPosition - 5;
        tempLine0 = "";
        tempLine1 = "Deceleration";
        delay(150);
        irValue = 0;
    }
    if (rightSpeed > 90 && leftSpeed > 90)
    {
        //////////////lcd.clear();
        tempLine0 = "Obstacle : " + String(distanceFwd);
        tempLine1 = "Avance";
    }
    else if (rightSpeed <= 90 && leftSpeed <= 90)
    {
        //////////////lcd.clear();
        tempLine0 = "Obstacle : " + String(distanceBwd);
        tempLine1 = "Recul";
    }
}

//
// Created by Cyril on 08/05/2020.
//

