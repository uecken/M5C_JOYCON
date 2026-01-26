// ボード別インクルード
#ifdef BOARD_M5STICKC
    #include <M5StickC.h>
#else
    #include <Arduino.h>
    #include <Wire.h>
#endif

#include <BleGamepad.h>
#include "JoyStick.h"

// I2Cピン定義（未定義の場合のデフォルト）
#ifndef I2C_SDA
    #define I2C_SDA 32  // M5StickC Grove
#endif
#ifndef I2C_SCL
    #define I2C_SCL 33  // M5StickC Grove
#endif

// BLEゲームパッド設定
#ifdef BOARD_M5STICKC
    BleGamepad bleGamepad("M5StickC JoyPad", "M5Stack", 100);
#else
    BleGamepad bleGamepad("XIAO JoyPad", "Seeed", 100);
#endif

// JoyStickインスタンス
JoyStick joystick;

// 前回値（変化検出用）- 中心値で初期化
int16_t prevX = 16383;
int16_t prevY = 16383;
bool prevButton = false;

// 画面表示更新
void updateDisplay(bool connected, int16_t x, int16_t y, bool btn) {
#ifdef BOARD_M5STICKC
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextSize(1);

    M5.Lcd.println("M5StickC JoyPad");

    if (connected) {
        M5.Lcd.setTextColor(GREEN);
        M5.Lcd.println("BLE: Connected");
    } else {
        M5.Lcd.setTextColor(YELLOW);
        M5.Lcd.println("BLE: Waiting...");
    }

    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.printf("Raw:%3d,%3d\n", joystick.getX(), joystick.getY());
    M5.Lcd.printf("Ctr:%3d,%3d\n", joystick.getCenterX(), joystick.getCenterY());
    M5.Lcd.printf("X: %6d\n", x);
    M5.Lcd.printf("Y: %6d\n", y);
    M5.Lcd.printf("Btn: %s\n", btn ? "ON" : "OFF");
#else
    // XIAO ESP32S3: シリアル出力のみ（LCDなし）
    (void)connected; (void)x; (void)y; (void)btn;  // 未使用警告抑制
#endif
}

void setup() {
#ifdef BOARD_M5STICKC
    // M5StickC初期化
    M5.begin();
    M5.Lcd.setRotation(3);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("Starting...");
#endif

    Serial.begin(115200);
#ifdef BOARD_M5STICKC
    Serial.println("M5StickC BLE JoyStick Controller");
#else
    Serial.println("XIAO ESP32S3 BLE JoyStick Controller");
#endif

    // I2C初期化（ボード別ピン）
    Wire.begin(I2C_SDA, I2C_SCL);
    joystick.begin(Wire);

    // JoyStickキャリブレーション
#ifdef BOARD_M5STICKC
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 20);
    M5.Lcd.setTextSize(1);
    M5.Lcd.println("Calibrating...");
    M5.Lcd.println("Keep stick neutral");
#endif
    Serial.println("Calibrating... Keep stick neutral");

    joystick.calibrate();

    Serial.printf("Calibration done: CenterX=%d, CenterY=%d\n",
                  joystick.getCenterX(), joystick.getCenterY());

#ifdef BOARD_M5STICKC
    M5.Lcd.printf("Center: %d,%d\n", joystick.getCenterX(), joystick.getCenterY());
#endif
    delay(500);

    // BLEゲームパッド初期化
    BleGamepadConfiguration config;
    config.setAutoReport(false);
    config.setControllerType(CONTROLLER_TYPE_GAMEPAD);
    config.setButtonCount(1);
    config.setHatSwitchCount(0);
    config.setWhichAxes(true, true, false, false, false, false, false, false);

    // 軸の範囲を0〜32767に設定（中心=16383）
    // ※ Androidは符号付き値を符号なしとして解釈するため
    config.setAxesMin(0);
    config.setAxesMax(32767);

    bleGamepad.begin(&config);

    Serial.println("BLE Config:");
    Serial.printf("  Axes range: 0 to 32767 (center=16383)\n");

    Serial.println("BLE Gamepad initialized");
    Serial.println("Waiting for connection...");

    updateDisplay(false, 0, 0, false);
}

// デバッグ用カウンタ
static unsigned long debugCount = 0;

void loop() {
#ifdef BOARD_M5STICKC
    M5.update();
#endif

    // JoyStick読み取り
    joystick.update();
    int16_t x = joystick.getAxisX();
    int16_t y = joystick.getAxisY();

    // ボタン取得（ボード別）
#ifdef BOARD_M5STICKC
    // M5StickCの本体ボタン(BtnA)をゲームパッドボタンとして使用
    bool button = M5.BtnA.isPressed();
#else
    // XIAO ESP32S3: JoyStickのボタンを使用
    bool button = joystick.isPressed();
#endif

    // 定期的なデバッグ出力（500ms間隔）
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug > 500) {
        Serial.printf("[%lu] Raw: X=%d Y=%d BtnRaw=%d | Ctr: X=%d Y=%d | Axis: X=%d Y=%d | Btn=%d | BLE=%d\n",
                      debugCount++,
                      joystick.getX(), joystick.getY(), joystick.getRawButton(),
                      joystick.getCenterX(), joystick.getCenterY(),
                      x, y, button,
                      bleGamepad.isConnected());
        lastDebug = millis();
    }

    // 値が変化した場合のみ処理
    bool changed = (x != prevX) || (y != prevY) || (button != prevButton);

    if (bleGamepad.isConnected()) {
        if (changed) {
            // 軸の値を設定
            bleGamepad.setX(x);
            bleGamepad.setY(y);

            // ボタン状態
            if (button && !prevButton) {
                bleGamepad.press(BUTTON_1);
            } else if (!button && prevButton) {
                bleGamepad.release(BUTTON_1);
            }

            bleGamepad.sendReport();
            Serial.printf("[SENT] X=%d Y=%d Btn=%d\n", x, y, button);
        }
    }

    // 画面更新（200ms間隔）
    static unsigned long lastDisplay = 0;
    if (millis() - lastDisplay > 200) {
        updateDisplay(bleGamepad.isConnected(), x, y, button);
        lastDisplay = millis();
    }

    prevX = x;
    prevY = y;
    prevButton = button;

    delay(10);
}
