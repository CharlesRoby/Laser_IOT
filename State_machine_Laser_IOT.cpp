// C++ code
#include <Arduino.h>
#include <HTTPClient.h>

// INPUTS ESP32
int sensorValue;
const int pin = 35; // Photoresistor
const int delai_state = 20;
const int delai_clr = 500;

// DECLARATION DES PINS
const int UNKNOWN_LED = 19;
const int WAITING_LED = 18;
const int CHECK_BIT1_LED = 5;
const int CHECK_BIT2_LED = 17;
const int CHECK_BIT3_LED = 16;
const int CHECK_BIT4_LED = 4;
const int CHECK_BIT5_LED = 2;
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
class Timer
{
    unsigned long _elapsed = 0;
    unsigned long startTime = 0;
    unsigned long elapsedTime = 0;

public:
    void start();
    unsigned long elapsed();
    void saveElapsedTime();
    unsigned long getSaveElapsedTime();
};

void Timer::start()
{
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

    bool checkState(State source, State target, bool condition, Timer *timer);

    State getCurrentState();
    State getOldState();
    void setOldState(State State);
};

FSM::FSM()
{
    mCurrentState = UNKNOWN;
    _oldstate = UNKNOWN;
}

// Function checkstate
bool FSM::checkState(State source, State target, bool condition = true, Timer* timer = NULL)
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
    if (source == this->mCurrentState && condition)
    {
        Serial.print("Change State : ");
        Serial.println(target);
        this->mCurrentState = target;

        if (NULL != timer)
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
int data;
int dataCheck;

bool receiveSignal()
{
    // Serial.print("Timer : ");
    // Serial.println(timer.elapsed());
    sensorValue = analogRead(pin);
    // Serial.print("Sensor Value : ");
    // Serial.println(sensorValue, DEC);

    if (sensorValue >= 500)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void saveData()
{
    sensorValue = analogRead(pin);
    if (sensorValue >= 500)
    {
        data = 1;
    }
    else
    {
        data = 0;
    }
}

bool checkParity()
{
    sensorValue = analogRead(pin);

    if (sensorValue >= 500)
    {
        dataCheck = 1;
    }
    else
    {
        dataCheck = 0;
    }

    if (data != dataCheck)
    {
        return true;
    }
    else
    {
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
    fsm.checkState(UNKNOWN, WAITING, true);
    fsm.checkState(WAITING, CHECK_BIT1, (timer.elapsed() > delai_state && receiveSignal()), &timer);

    fsm.checkState(CHECK_BIT1, CHECK_BIT2, (timer.elapsed() > delai_state && receiveSignal()), &timer);
    fsm.checkState(CHECK_BIT1, WAITING, (timer.elapsed() > delai_clr && !receiveSignal()), &timer);

    fsm.checkState(CHECK_BIT2, CHECK_BIT3, (timer.elapsed() > delai_state && !receiveSignal()), &timer);
    fsm.checkState(CHECK_BIT2, WAITING, (timer.elapsed() > delai_clr && !receiveSignal()), &timer);

    fsm.checkState(CHECK_BIT3, CHECK_BIT4, (timer.elapsed() > delai_state && receiveSignal()), &timer);
    fsm.checkState(CHECK_BIT3, WAITING, (timer.elapsed() > delai_clr && !receiveSignal()), &timer);

    fsm.checkState(CHECK_BIT4, CHECK_BIT5, (timer.elapsed() > delai_state), &timer);
    fsm.checkState(CHECK_BIT4, WAITING, (timer.elapsed() > delai_clr && !receiveSignal()), &timer);

    fsm.checkState(CHECK_BIT5, HIT, (timer.elapsed() > delai_state && checkParity()), &timer); // Si le BIT de parite est correct (true)
    fsm.checkState(CHECK_BIT5, WAITING, (timer.elapsed() > delai_state && !checkParity()), &timer); // Si le BIT de parite est mauvais (false)
    fsm.checkState(CHECK_BIT5, WAITING, (timer.elapsed() > delai_clr && !receiveSignal()), &timer);

    fsm.checkState(HIT, WAITING, timer.elapsed() > 3000, &timer);
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
        saveData();
        break;
    case CHECK_BIT5:
        LedChange(UNKNOWN_LED, true);
        LedChange(WAITING_LED, true);
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
