# M5StickC BLE JoyStick コントローラー

## プロジェクト概要

Grove端子にJoyStickを接続したM5StickCをBluetooth BLEゲームパッドとして動作させ、Raspberry Pi 4で使用する。

## システム構成

```
┌─────────────────┐      BLE      ┌─────────────────┐
│   M5StickC      │ ◄──────────► │  Raspberry Pi 4 │
│  + JoyStick     │              │  (JoyStickアプリ) │
│   (Grove I2C)   │              │                 │
└─────────────────┘              └─────────────────┘
```

## ハードウェア

- **M5StickC**: ESP32-PICO-D4搭載
- **JoyStickユニット**: M5Stack Grove JoyStick (I2C接続, アドレス: 0x52)
  - X軸, Y軸: 0-255 (中心: 128)
  - ボタン: 押下で0

## 使用ライブラリ

| ライブラリ | 用途 |
|-----------|------|
| M5StickC | デバイス制御 |
| ESP32-BLE-Gamepad | BLEゲームパッド機能 |
| NimBLE-Arduino | BLE通信（効率的な実装） |

## 参考リポジトリ

- [M5StickC_Gamepad](https://github.com/esikora/M5StickC_Gamepad) - 本プロジェクトの参考実装
- [ESP32-BLE-Gamepad](https://github.com/lemmingDev/ESP32-BLE-Gamepad) - BLEゲームパッドライブラリ

## 技術メモ

- **PlatformIO espressif32 @ 6.3.2** を使用（Arduino ESP32 v2.0.9）
- Arduino Framework v3系はAPI変更が大きいため使用不可
- M5StickC JoyStickはI2C通信（Wire）- Grove端子 SDA=32, SCL=33
- BLE HID Gamepadとして認識される
- NimBLE-Arduinoライブラリ使用（効率的なBLE実装）

### 軸の値範囲

| 項目 | 値 |
|------|-----|
| 最小値 | 0 |
| 中心値 | 16383 |
| 最大値 | 32767 |
| デッドゾーン | 15 |

**注意**: HIDレポートは符号なし値として解釈されるため、-32767〜32767ではなく0〜32767を使用。

### ボタン

- JoyStickのボタンは常に0を返すため、**M5StickC本体のBtnA**をゲームパッドボタンとして使用

## テスト手順

### Android (adb) でのテスト

```bash
# デバイス情報確認（軸の範囲とニュートラル値）
adb shell "getevent -p" | head -10

# 期待される出力:
# name: "M5StickC JoyPad"
# ABS: value 16383, min 0, max 32767

# リアルタイム入力確認
adb shell "getevent -l /dev/input/event4"
```

### テスト項目

| # | テスト内容 | 期待結果 |
|---|-----------|---------|
| 1 | ニュートラル時の軸値 | X=16383, Y=16383 |
| 2 | スティック右 | X > 16383 |
| 3 | スティック左 | X < 16383 |
| 4 | スティック上 | Y < 16383 |
| 5 | スティック下 | Y > 16383 |
| 6 | BtnA押下 | KEY_BTN_A イベント発生 |
| 7 | デッドゾーン | 中心付近で値が16383固定 |

## ファイル構成

```
m5c_joycon/
├── CLAUDE.md          # 本ファイル（設計資料）
├── platformio.ini     # PlatformIO設定
├── src/
│   └── main.cpp       # メインコード
└── include/
    └── JoyStick.h     # JoyStick読み取りクラス
```

## ビルド手順

```bash
# PlatformIOでビルド
pio run

# アップロード
pio run -t upload

# シリアルモニタ
pio device monitor
```
