#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <Wire.h>

// M5Stack JoyStick I2Cアドレス
#define JOYSTICK_V1_ADDR  0x52  // JoyStick Unit 旧型: 8bit ADC (0-255)
#define JOYSTICK_HAT_ADDR 0x54  // JoyStick Hat (MiniJoyC): 8bit ADC
#define JOYSTICK_V2_ADDR  0x63  // JoyStick Unit RGB LED付き: 16bit ADC (0-65535)

// デッドゾーン（中心付近のノイズ除去）
#define JOYSTICK_DEADZONE_V1 15    // 8bit用
#define JOYSTICK_DEADZONE_V2 1000  // 16bit用

// JoyStickタイプ
enum JoyStickType {
    JOYSTICK_TYPE_UNKNOWN = 0,
    JOYSTICK_TYPE_V1,   // 0x52, 8bit (JoyStick Unit)
    JOYSTICK_TYPE_HAT,  // 0x54, 8bit (JoyStick Hat)
    JOYSTICK_TYPE_V2    // 0x63, 16bit (JoyStick Unit RGB)
};

class JoyStick {
public:
    JoyStick() : _x(0), _y(0), _button(false), _rawButton(1),
                 _centerX(0), _centerY(0), _calibrated(false),
                 _type(JOYSTICK_TYPE_UNKNOWN), _addr(0) {}

    void begin(TwoWire &wire = Wire) {
        _wire = &wire;
        // 自動検出
        detectType();
    }

    // JoyStickタイプを自動検出
    bool detectType() {
        // V1 (0x52) をチェック - JoyStick Unit
        _wire->beginTransmission(JOYSTICK_V1_ADDR);
        if (_wire->endTransmission() == 0) {
            _type = JOYSTICK_TYPE_V1;
            _addr = JOYSTICK_V1_ADDR;
            _centerX = 128;
            _centerY = 128;
            Serial.printf("JoyStick Unit V1 detected (0x%02X, 8bit)\n", _addr);
            return true;
        }

        // Hat (0x54) をチェック - JoyStick Hat (MiniJoyC)
        _wire->beginTransmission(JOYSTICK_HAT_ADDR);
        if (_wire->endTransmission() == 0) {
            _type = JOYSTICK_TYPE_HAT;
            _addr = JOYSTICK_HAT_ADDR;
            _centerX = 128;
            _centerY = 128;
            Serial.printf("JoyStick Hat detected (0x%02X, 8bit)\n", _addr);
            return true;
        }

        // V2 (0x63) をチェック - JoyStick Unit RGB LED付き
        _wire->beginTransmission(JOYSTICK_V2_ADDR);
        if (_wire->endTransmission() == 0) {
            _type = JOYSTICK_TYPE_V2;
            _addr = JOYSTICK_V2_ADDR;
            _centerX = 32768;
            _centerY = 32768;
            Serial.printf("JoyStick Unit V2 detected (0x%02X, 16bit)\n", _addr);
            return true;
        }

        Serial.println("JoyStick not detected!");
        return false;
    }

    JoyStickType getType() const { return _type; }
    uint8_t getAddress() const { return _addr; }

    // 起動時のキャリブレーション（現在位置を中心とする）
    void calibrate() {
        if (_type == JOYSTICK_TYPE_UNKNOWN) return;

        // 複数回読み取って平均を取る
        int32_t sumX = 0, sumY = 0;
        const int samples = 10;

        for (int i = 0; i < samples; i++) {
            update();
            sumX += _x;
            sumY += _y;
            delay(10);
        }

        _centerX = sumX / samples;
        _centerY = sumY / samples;
        _calibrated = true;
    }

    void update() {
        if (_type == JOYSTICK_TYPE_V1 || _type == JOYSTICK_TYPE_HAT) {
            updateV1();  // V1とHatは同じ8bitフォーマット
        } else if (_type == JOYSTICK_TYPE_V2) {
            updateV2();
        }
    }

    uint8_t getRawButton() const { return _rawButton; }

    // 生値を取得（V1: 0-255, V2: 0-65535）
    uint16_t getX() const { return _x; }
    uint16_t getY() const { return _y; }
    uint16_t getCenterX() const { return _centerX; }
    uint16_t getCenterY() const { return _centerY; }
    bool isPressed() const { return _button; }

    // 0〜32767に変換（中心=16383、キャリブレーション済み、デッドゾーン適用）
    // ※ HIDは符号なしとして解釈されるため、この範囲を使用
    int16_t getAxisX() const {
        // X軸は反転
        return convertAxis(_x, _centerX, true);
    }

    int16_t getAxisY() const {
        return convertAxis(_y, _centerY, false);
    }

private:
    TwoWire *_wire;
    uint16_t _x;
    uint16_t _y;
    bool _button;
    uint8_t _rawButton;
    uint16_t _centerX;
    uint16_t _centerY;
    bool _calibrated;
    JoyStickType _type;
    uint8_t _addr;

    // V1 (0x52): 3バイト読み取り (X:8bit, Y:8bit, Button:8bit)
    void updateV1() {
        _wire->requestFrom(_addr, (uint8_t)3);
        if (_wire->available() >= 3) {
            _x = _wire->read();         // X軸: 0-255
            _y = _wire->read();         // Y軸: 0-255
            _rawButton = _wire->read(); // ボタン生値
            _button = (_rawButton == 0); // 0=押下
        }
    }

    // V2 (0x63): 5バイト読み取り (X:16bit, Y:16bit, Button:8bit)
    void updateV2() {
        _wire->requestFrom(_addr, (uint8_t)5);
        if (_wire->available() >= 5) {
            uint8_t xL = _wire->read();
            uint8_t xH = _wire->read();
            uint8_t yL = _wire->read();
            uint8_t yH = _wire->read();
            _rawButton = _wire->read();

            _x = (uint16_t)xL | ((uint16_t)xH << 8);  // Little endian
            _y = (uint16_t)yL | ((uint16_t)yH << 8);
            _button = (_rawButton == 0); // 0=押下
        }
    }

    // 中心値からの相対値を計算（0〜32767、中心=16383）
    int16_t convertAxis(uint16_t value, uint16_t center, bool invert) const {
        int32_t diff = (int32_t)value - (int32_t)center;

        // 反転処理
        if (invert) {
            diff = -diff;
        }

        // デッドゾーン
        int32_t deadzone = (_type == JOYSTICK_TYPE_V2) ? JOYSTICK_DEADZONE_V2 : JOYSTICK_DEADZONE_V1;

        if (abs(diff) < deadzone) {
            return 16383;  // 中心値
        }

        // デッドゾーン外の値を調整
        if (diff > 0) {
            diff -= deadzone;
        } else {
            diff += deadzone;
        }

        // スケーリング
        int32_t maxRange;
        if (_type == JOYSTICK_TYPE_V2) {
            maxRange = 32767 - deadzone;  // 16bit: 中心から約32767
        } else {
            maxRange = 127 - deadzone;  // 8bit: 中心から約127 (V1, HAT)
        }

        // 中心(16383)を基準に±16383の範囲でスケーリング
        int32_t scaled = 16383 + (diff * 16383) / maxRange;

        // クリップ（0〜32767）
        if (scaled > 32767) scaled = 32767;
        if (scaled < 0) scaled = 0;

        return (int16_t)scaled;
    }
};

#endif // JOYSTICK_H
