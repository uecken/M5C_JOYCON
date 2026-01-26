# ESP32 Bluetooth機能比較

## 概要

本ドキュメントでは、ESP32シリーズの主要チップにおけるBluetooth機能を比較します。

## 比較対象チップ

1. **ESP32-WROOM-32** - オリジナルESP32モジュール
2. **ESP32-S3** - AI/ML機能強化版
3. **ESP32-C6** - Wi-Fi 6対応版

## Bluetooth機能比較表

| 機能 | ESP32-WROOM-32 | ESP32-S3 | ESP32-C6 |
|------|----------------|----------|----------|
| **Classic Bluetooth** | Yes | No | No |
| **BLE (Bluetooth Low Energy)** | Yes | Yes | Yes |
| **BLEバージョン** | BLE 4.2 | BLE 5.0 | BLE 5.0 |
| **デュアルモード対応** | Yes | No | No |
| **Long Range (Coded PHY)** | No | Yes | Yes |
| **2M PHY** | No | Yes | Yes |
| **LE拡張アドバタイジング** | No | Yes | Yes |
| **同時接続数** | 最大9 | 最大9 | 最大8 |
| **Thread/Zigbee** | No | No | Yes |

## 各チップの詳細

### ESP32-WROOM-32（オリジナル）

- **CPU**: Xtensa LX6 デュアルコア @ 240MHz
- **Bluetooth特徴**:
  - Classic BluetoothとBLEの両方をサポート（デュアルモード）
  - Bluetooth v4.2 BR/EDR および BLE対応
  - A2DP、AVRCP、SPPなどのClassicプロファイルが使用可能

### ESP32-S3

- **CPU**: Xtensa LX7 デュアルコア @ 240MHz
- **Bluetooth特徴**:
  - BLE 5.0のみサポート（Classic Bluetoothは非対応）
  - Long Range（最大4倍の通信距離）
  - 2倍の通信速度（2M PHY）
  - LE Audio対応の可能性

### ESP32-C6

- **CPU**: RISC-V シングルコア @ 160MHz
- **Bluetooth特徴**:
  - BLE 5.0のみサポート（Classic Bluetoothは非対応）
  - IEEE 802.15.4（Thread/Zigbee/Matter）対応
  - Wi-Fi 6（802.11ax）対応
  - 低消費電力設計

## BLE HID Gamepadプロジェクト向け推奨

### 推奨: ESP32-WROOM-32（オリジナル）

BLE HID Gamepadプロジェクトには、**ESP32-WROOM-32（オリジナル）** を推奨します。

#### 推奨理由

1. **ライブラリの成熟度**
   - ESP32-BLE-Gamepadライブラリが最も安定して動作
   - 多くの実績とコミュニティサポートがある
   - NimBLE-Arduinoとの互換性が確認済み

2. **互換性**
   - 最も多くのデバイス（PC、スマートフォン、Raspberry Pi等）との接続実績
   - HIDプロファイルの動作が安定

3. **開発の容易さ**
   - 豊富なサンプルコードとドキュメント
   - トラブルシューティング情報が充実

4. **Classic Bluetoothオプション**
   - 必要に応じてClassic Bluetooth HIDへの切り替えが可能
   - 一部の古いデバイスとの互換性を確保

#### ESP32-S3/C6を選ぶ場合

以下の要件がある場合は、ESP32-S3またはESP32-C6を検討してください：

- **長距離通信が必要**: BLE 5.0のLong Range機能
- **高速データ転送**: 2M PHYによる高速通信
- **低消費電力重視**: ESP32-C6の省電力設計
- **将来性**: LE Audioなど新機能への対応

### 注意事項

- ESP32-S3/C6ではESP32-BLE-Gamepadライブラリの互換性を事前に確認してください
- Arduino ESP32 Core v2.x系での動作確認を推奨（v3.x系はAPI変更が大きい）
- M5StickCはESP32-PICO-D4（ESP32-WROOM-32相当）を搭載しています

## 参考資料

- [Espressif ESP32シリーズ比較](https://www.espressif.com/en/products/socs)
- [ESP32-BLE-Gamepad](https://github.com/lemmingDev/ESP32-BLE-Gamepad)
- [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino)
