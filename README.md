# M5StickC BLE JoyStick Controller

M5StickC + Grove JoyStick を BLE ゲームパッドとして動作させるプロジェクト。

## 概要

Grove端子に接続したJoyStickの入力をBluetooth Low Energy (BLE) HID Gamepadとして送信します。
Raspberry Pi、Android、PCなどで汎用ゲームパッドとして認識されます。

## ハードウェア

- **M5StickC** (ESP32-PICO-D4)
- **M5Stack Grove JoyStick Unit** (I2C: 0x52)

## 接続

```
M5StickC Grove端子 ─── JoyStick Unit
  SDA (GPIO32)          SDA
  SCL (GPIO33)          SCL
  3.3V                  VCC
  GND                   GND
```

## 機能

- BLE HID Gamepad として認識
- 起動時に自動キャリブレーション
- デッドゾーン処理（中心付近のノイズ除去）
- M5StickC本体のBtnAをゲームパッドボタンとして使用

## 軸の値範囲

| 項目 | 値 |
|------|-----|
| 最小値 | 0 |
| 中心値 | 16383 |
| 最大値 | 32767 |

## ビルド

PlatformIO を使用:

```bash
# ビルド
pio run

# アップロード
pio run -t upload

# シリアルモニタ
pio device monitor
```

## 使用ライブラリ

- M5StickC
- ESP32-BLE-Gamepad
- NimBLE-Arduino

## 動作確認済み環境

- Android (Gamepad Tester アプリ)
- Raspberry Pi 4

## 参考

- [M5StickC_Gamepad](https://github.com/esikora/M5StickC_Gamepad)
- [ESP32-BLE-Gamepad](https://github.com/lemmingDev/ESP32-BLE-Gamepad)

## License

MIT
