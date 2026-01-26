#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <Wire.h>

// M5Stack Grove JoyStick I2Cアドレス
#define JOYSTICK_I2C_ADDR 0x52

// デッドゾーン（中心付近のノイズ除去）
#define JOYSTICK_DEADZONE 15

class JoyStick {
public:
    JoyStick() : _x(128), _y(128), _button(false), _rawButton(1),
                 _centerX(128), _centerY(128), _calibrated(false) {}

    void begin(TwoWire &wire = Wire) {
        _wire = &wire;
    }

    // 起動時のキャリブレーション（現在位置を中心とする）
    void calibrate() {
        // 複数回読み取って平均を取る
        int sumX = 0, sumY = 0;
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
        _wire->requestFrom(JOYSTICK_I2C_ADDR, 3);
        if (_wire->available() >= 3) {
            _x = _wire->read();       // X軸: 0-255
            _y = _wire->read();       // Y軸: 0-255
            _rawButton = _wire->read(); // 生値を保存
            _button = (_rawButton == 0); // ボタン: 0=押下
        }
    }

    uint8_t getRawButton() const { return _rawButton; }

    // 0-255の生値を取得
    uint8_t getX() const { return _x; }
    uint8_t getY() const { return _y; }
    uint8_t getCenterX() const { return _centerX; }
    uint8_t getCenterY() const { return _centerY; }
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
    uint8_t _x;
    uint8_t _y;
    bool _button;
    uint8_t _rawButton;
    uint8_t _centerX;
    uint8_t _centerY;
    bool _calibrated;

    // 中心値からの相対値を計算（0〜32767、中心=16383）
    int16_t convertAxis(uint8_t value, uint8_t center, bool invert) const {
        int16_t diff = (int16_t)value - (int16_t)center;

        // 反転処理
        if (invert) {
            diff = -diff;
        }

        // デッドゾーン内は中心値を返す
        if (abs(diff) < JOYSTICK_DEADZONE) {
            return 16383;  // 中心値（32767 / 2）
        }

        // デッドゾーン外の値を調整
        if (diff > 0) {
            diff -= JOYSTICK_DEADZONE;
        } else {
            diff += JOYSTICK_DEADZONE;
        }

        // スケーリング：中心からの最大距離は約127
        // デッドゾーン分を引いた後の最大値は約112
        int16_t maxRange = 127 - JOYSTICK_DEADZONE;

        // 中心(16383)を基準に±16383の範囲でスケーリング
        int32_t scaled = 16383 + ((int32_t)diff * 16383) / maxRange;

        // クリップ（0〜32767）
        if (scaled > 32767) scaled = 32767;
        if (scaled < 0) scaled = 0;

        return (int16_t)scaled;
    }
};

#endif // JOYSTICK_H
