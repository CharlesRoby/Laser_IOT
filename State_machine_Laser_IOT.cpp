// C++ code
#include <Arduino.h>
#include <HTTPClient.h>

// INPUTS ESP32
int pin = 35;    // Photoresistor
int sensorValue;
int crossed = 0;
int last = 0;

// DECLARATION DES PINS
const int UNKNOWN_LED = 19;
const int WAITING_LED = 18;
const int CHECK_BIT1_LED = 5;
const int CHECK_BIT2_LED = 17;
const int CHECK_BIT3_LED = 16;
const int CHECK_BIT4_LED = 4;
const int CHECK_BIT5_LED = 0;
const int HIT_LED = 21;

// const int leds[8] = {19, 18, 5, 17, 16, 4, 0, 21};

bool terrianAerian = false;

// const char *ssid = "HONOR 9X Lite"; // Main Router
// const char *password = "test0123";  // Main Router Password

// const char *url = "http://storm5301.hub.ubeac.io/IK7YUJTHGRBVFDESRCX";
// String payload_pattern = "[{\"id\": \"MyESP\", \"sensors\": [{\"id\": \"Laser Crossed\", \"value\": $laser$}]}]";

// Différent state
enum State
{
    UNKNOWN,
    WAITING,
    CHECK_BIT1,
    CHECK_BIT2,
    CHECK_BIT3,
    CHECK_BIT4,
    CHECK_BIT5,
    HIT
};

// Class Timer
class Timer {
	unsigned long _elapsed = 0;
	unsigned long startTime = 0;
  	unsigned long elapsedTime = 0;
public:

	void start();
	unsigned long elapsed();
  	void saveElapsedTime();
  	unsigned long getSaveElapsedTime();
};

void Timer::start() {
	startTime = millis();
}

unsigned long Timer::elapsed()
{
	_elapsed = millis() - startTime;
	return _elapsed;
}

void Timer::saveElapsedTime()
{
	_elapsed = millis() - startTime;
  	elapsedTime = _elapsed;
}

unsigned long Timer::getSaveElapsedTime() { return elapsedTime; }



// Class FSM
class FSM
{
    State mCurrentState;
  	State _oldstate;

public:
    FSM();

    bool checkState(State source, State target, bool condition_millis, bool condition_signal, Timer* timer);
    
    State getCurrentState();
  	State getOldState();
  	void setOldState(State State);
};

FSM::FSM() {
    mCurrentState = UNKNOWN;
  	_oldstate = UNKNOWN;
}

// Function checkstate
bool FSM::checkState(State source, State target, bool condition_millis = true, bool condition_signal = true, Timer* timer = NULL)
{

    // if (source == mCurrentState && condition_millis && condition_signal)
    // {
    //     if (timer != NULL)
    //     {
    //         timer->start();
    //     }
    //     mCurrentState = target;
    // }

    bool etat = false;
	if (source == this->mCurrentState && condition_millis && condition_signal)
	{
		Serial.println("Change State : " + target);
		this->mCurrentState = target;
		
      	if(NULL != timer)
      	{
      	  timer->start();
      	}
      
      	etat = true;
	}
	return etat;
}

State FSM::getCurrentState() { return mCurrentState; }
State FSM::getOldState() { return _oldstate; }
void FSM::setOldState(State State) { _oldstate = State; }

FSM fsm;
Timer timer;



bool receiveSignal()
{
    Serial.print("Timer : ");
    Serial.println(timer.elapsed());
    sensorValue = analogRead(pin);
    Serial.print("Sensor Value : ");
    Serial.println(sensorValue, DEC);
    last = crossed;

    if (sensorValue >= 500)
    {
        crossed = 1;
        timer.start();
        return true;
    }
    else
    {
        crossed = 0;
        return false;
    }
}


void LedChange(int numled, bool ledstatus = false)
{
    digitalWrite(numled, ledstatus);
}


void setup()
{
    // INITIALISATION DU SERIAL MONITOR
    Serial.begin(9600);

    // INITIALISATION DES LEDS
    pinMode(UNKNOWN_LED, OUTPUT);
    pinMode(WAITING_LED, OUTPUT);
    pinMode(CHECK_BIT1_LED, OUTPUT);
    pinMode(CHECK_BIT2_LED, OUTPUT);
    pinMode(CHECK_BIT3_LED, OUTPUT);
    pinMode(CHECK_BIT4_LED, OUTPUT);
    pinMode(CHECK_BIT5_LED, OUTPUT);
    pinMode(HIT_LED, OUTPUT);

    // INITIALISATION DU CAPTEUR
    pinMode(sensorValue, INPUT);
}

// RunFsm passage d'états
void RunFsm()
{
    fsm.checkState(UNKNOWN, WAITING, true, true);
    fsm.checkState(WAITING, CHECK_BIT1, (timer.elapsed() > 500 && receiveSignal()), &timer);
    
    fsm.checkState(CHECK_BIT1, CHECK_BIT2, (timer.elapsed() > 500 && receiveSignal()), &timer);
    //fsm.checkState(CHECK_BIT1, WAITING, (timer.elapsed() > 1500 && !receiveSignal()), &timer);

    fsm.checkState(CHECK_BIT2, CHECK_BIT3, (timer.elapsed() > 500 && !receiveSignal()), &timer);
    //fsm.checkState(CHECK_BIT2, WAITING, (timer.elapsed() > 1500 && !receiveSignal()), &timer);
    
    fsm.checkState(CHECK_BIT3, CHECK_BIT4, (timer.elapsed() > 500 && receiveSignal()), &timer);
    //fsm.checkState(CHECK_BIT3, WAITING, (timer.elapsed() > 1500 && !receiveSignal()), &timer);
    
    fsm.checkState(CHECK_BIT4, CHECK_BIT5, (timer.elapsed() > 500 && receiveSignal()), &timer);
    //fsm.checkState(CHECK_BIT4, WAITING, (timer.elapsed() > 1500 && !receiveSignal()), &timer);
    
    fsm.checkState(CHECK_BIT5, HIT, (timer.elapsed() > 500 && receiveSignal()), &timer);
    //fsm.checkState(CHECK_BIT5, WAITING, (timer.elapsed() > 1500 && !receiveSignal()), &timer);

    fsm.checkState(HIT, WAITING, (timer.elapsed() > 500 && receiveSignal()), &timer);
}


void loop()
{
    RunFsm();

    // Code switch
    switch (fsm.getCurrentState())
    {
    case UNKNOWN:
        LedChange(UNKNOWN_LED, true);
        LedChange(WAITING_LED, false);
        LedChange(CHECK_BIT1_LED, false);
        LedChange(CHECK_BIT2_LED, false);
        LedChange(CHECK_BIT3_LED, false);
        LedChange(CHECK_BIT4_LED, false);
        LedChange(CHECK_BIT5_LED, false);
        LedChange(HIT_LED, false);
        break;
    case WAITING:
        LedChange(UNKNOWN_LED, true);
        LedChange(WAITING_LED, true);
        LedChange(CHECK_BIT1_LED, false);
        LedChange(CHECK_BIT2_LED, false);
        LedChange(CHECK_BIT3_LED, false);
        LedChange(CHECK_BIT4_LED, false);
        LedChange(CHECK_BIT5_LED, false);
        LedChange(HIT_LED, false);
        break;
    case CHECK_BIT1:
        LedChange(UNKNOWN_LED, true);
        LedChange(WAITING_LED, true);
        LedChange(CHECK_BIT1_LED, true);
        LedChange(CHECK_BIT2_LED, false);
        LedChange(CHECK_BIT3_LED, false);
        LedChange(CHECK_BIT4_LED, false);
        LedChange(CHECK_BIT5_LED, false);
        LedChange(HIT_LED, false);
        break;
    case CHECK_BIT2:
        LedChange(UNKNOWN_LED, true);
        LedChange(WAITING_LED, true);
        LedChange(CHECK_BIT1_LED, true);
        LedChange(CHECK_BIT2_LED, true);
        LedChange(CHECK_BIT3_LED, false);
        LedChange(CHECK_BIT4_LED, false);
        LedChange(CHECK_BIT5_LED, false);
        LedChange(HIT_LED, false);
        break;
    case CHECK_BIT3:
        LedChange(UNKNOWN_LED, true);
        LedChange(WAITING_LED, true);
        LedChange(CHECK_BIT1_LED, true);
        LedChange(CHECK_BIT2_LED, true);
        LedChange(CHECK_BIT3_LED, true);
        LedChange(CHECK_BIT4_LED, false);
        LedChange(CHECK_BIT5_LED, false);
        LedChange(HIT_LED, false);
        break;
    case CHECK_BIT4:
        LedChange(UNKNOWN_LED, true);
        LedChange(WAITING_LED, true);
        LedChange(CHECK_BIT1_LED, true);
        LedChange(CHECK_BIT2_LED, true);
        LedChange(CHECK_BIT3_LED, true);
        LedChange(CHECK_BIT4_LED, true);
        LedChange(CHECK_BIT5_LED, false);
        LedChange(HIT_LED, false);
        break;
    case CHECK_BIT5:
        LedChange(UNKNOWN_LED, true);
        LedChange(WAITING_LED, false);
        LedChange(CHECK_BIT1_LED, true);
        LedChange(CHECK_BIT2_LED, true);
        LedChange(CHECK_BIT3_LED, true);
        LedChange(CHECK_BIT4_LED, true);
        LedChange(CHECK_BIT5_LED, true);
        LedChange(HIT_LED, false);
        break;
    case HIT:
        LedChange(UNKNOWN_LED, true);
        LedChange(WAITING_LED, true);
        LedChange(CHECK_BIT1_LED, true);
        LedChange(CHECK_BIT2_LED, true);
        LedChange(CHECK_BIT3_LED, true);
        LedChange(CHECK_BIT4_LED, true);
        LedChange(CHECK_BIT5_LED, true);
        LedChange(HIT_LED, true);
        break;
    }
}

