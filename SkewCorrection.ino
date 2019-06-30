//Copyright (C) 2019 sion//

#include "NineAxesMotion.h"        //Contains the bridge code between the API and the Arduino Environment
#include <Wire.h>

//サーボモーターの制御に使用する為の定数、変数です。
const int TARGET_PIN_NUMBER = 2;//PWM信号を出力するIOポート番号です。
const int DEG_0_MSEC = 600; // msec.
const int DEG_180_MSEC = 2350; // msec.
int microSec = DEG_0_MSEC;

//ジャイロセンサーより、傾き値を取得する為の定数、変数です。
NineAxesMotion mySensor;         //Object that for the sensor 
const int STREAM_PERIOD = 20;          //To stream at 50Hz without using additional timers (time period(ms) =1000/frequency(Hz))
unsigned long lastStreamTime = 0;     //To store the last streamed time stamp

//コンソール出力に関する設定をする為の定数です。
const int SERIAL_PORT_CONSOLE_DEBUG = 115200;

//初回のみ実行します。
void setup() {
    SerialConsole_Init(SERIAL_PORT_CONSOLE_DEBUG);
    SensorOfRotate_Init();
    ServoMotor_Init(TARGET_PIN_NUMBER);
}

//ポーリング実行されます。
void loop() { 
    float receiveRotate = GetRotateFromSensor();
    ExecuteSygnalToServo(receiveRotate);
}

//傾き値の取得に関する設定をする関数です。
void SensorOfRotate_Init(){
    mySensor.initSensor();                              //ジャイロセンサーの初期化
    mySensor.setOperationMode(OPERATION_MODE_NDOF);
    mySensor.setUpdateMode(MANUAL);
}

//コンソール出力に関する設定をする関数です。
void SerialConsole_Init(int portNumber){
    Serial.begin(portNumber);
    I2C.begin();
}

//サーボモーターに関する設定をする関数です。
void ServoMotor_Init(int pinNumber){
    pinMode( pinNumber, OUTPUT );//PWM信号を出力する為のアナログピンを指定します。
}

//ジャイロセンサーから傾き値を取得します。
float GetRotateFromSensor(){
    if ((millis() - lastStreamTime) >= STREAM_PERIOD){
        lastStreamTime = millis();
        mySensor.updateEuler();        //センサー測定値取得オブジェクトの内容を更新します。
        mySensor.updateCalibStatus();  
    }
    float receivedRotated = mySensor.readEulerPitch();//updateRotate
    OutputLog(" P: ", receivedRotated, "deg ", "");
    return receivedRotated;
}

//指定された角度にサーボモータが動作する様に、PWM信号を発信します。
void ExecuteSygnalToServo(float sourceDeg){
    delay(10);
    float deg = sourceDeg + 90;//測定値が0±90度に対してサーボが0~180度なので+90度オフセットする。
    if ( deg >= 0 && deg <= 180 ){//サーボへの指令値が閾値内であれば反映する。
        microSec = DEG_0_MSEC + deg / 180.0 * ( DEG_180_MSEC - DEG_0_MSEC );//角度⇒PWM変換(ヽ''ω`)ぱうむ！
    }
    if ( microSec >= DEG_0_MSEC ) {//サーボへ指令値を適用する。
        digitalWrite( 2, HIGH );
        delayMicroseconds( microSec ); // ON
        digitalWrite( 2, LOW );
        delayMicroseconds( 10000 ); // OFF
        delayMicroseconds( 10000 - microSec ); // OFF
    }
}

//ログテキストを設定されたシリアルポートを経由し、コンソール画面に表示します。
void OutputLog(const char* key, float value, const char* unit, const char* comment){
    Serial.print(key);
    Serial.print(value);
    Serial.print(unit);
    Serial.print(comment);
    Serial.println();//改行
}