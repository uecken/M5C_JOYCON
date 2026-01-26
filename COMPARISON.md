# プロジェクト比較資料

## M5StickC_Gamepad（参考） vs m5c_joycon（新規）

### 1. ライブラリ構成の違い

| 項目 | M5StickC_Gamepad（参考） | m5c_joycon（新規） |
|------|-------------------------|-------------------|
| BLEライブラリ | ESP32標準BLE（手動HID実装） | ESP32-BLE-Gamepad + NimBLE |
| Platform版 | `espressif32`（バージョン指定なし） | `espressif32 @ 6.3.2` |
| lib_deps | M5StickCのみ | M5StickC, ESP32-BLE-Gamepad, NimBLE-Arduino |

### 2. JoyStick処理の違い

#### 参考プロジェクト（M5StickC_GamepadIO.h:157-182）
```cpp
// 中心値：ハードコード
uint8_t joyRawCenterX_ = 122;
uint8_t joyRawCenterY_ = 122;

// 係数（Y軸反転）
uint8_t joyFactorX_ = 1;
uint8_t joyFactorY_ = -1;

// 正規化計算
joyNormX_ = joyFactorX_ * (joyRawX_ - joyRawCenterX_);
joyNormY_ = joyFactorY_ * (joyRawY_ - joyRawCenterY_);

// BLE送信時（8ビットシフト）
GamepadBLE::StickAxis_t joyScaledX = pGamepadIO->getJoyNormX() << 8;
```

#### 新規プロジェクト（修正前）
```cpp
// 単純な0-255から-32767〜32767へのマッピング
return map(_x, 0, 255, -32767, 32767);
```

### 3. 重要な発見事項

1. **中心値**: 参考プロジェクトでは122をハードコード（TODOコメントで動的決定を検討）
2. **Y軸反転**: 参考では `joyFactorY_ = -1` で反転
3. **値の範囲**: 参考では-128〜127の8bit値を8ビットシフトして16bit化
4. **JoyStick押下**: `joyPressed_` はI2Cの3バイト目をそのまま使用（0=押下ではない）

### 4. 修正すべき点

| 問題 | 解決策 |
|------|--------|
| 中心値が128固定 | キャリブレーションで実測値を使用（既に実装済み） |
| Y軸の向きが逆かも | `joyFactorY_ = -1` を検討 |
| デッドゾーン | 15程度のデッドゾーン実装（既に実装済み） |
| ボタン検出 | 参考では `joyPressed_` をそのまま使用 |

### 5. ファイル構成比較

```
M5StickC_Gamepad/                    m5c_joycon/
├── src/                             ├── src/
│   ├── M5StickC_GamepadApp.cpp      │   └── main.cpp
│   ├── M5StickC_GamepadIO.cpp       │
│   ├── GamepadBLE.cpp               │
│   ├── AXP192_BLEService.cpp        │
│   ├── BLE2901.cpp                  │
│   └── M5StickC_PowerManagement.cpp │
├── include/                         ├── include/
│   ├── M5StickC_GamepadIO.h         │   └── JoyStick.h
│   ├── GamepadBLE.h                 │
│   ├── HIDDescriptor.h              │
│   ├── AXP192_BLEService.h          │
│   ├── BLE2901.h                    │
│   └── M5StickC_PowerManagement.h   │
└── platformio.ini                   └── platformio.ini
```

**参考プロジェクトは複雑だが、本プロジェクトはESP32-BLE-Gamepadライブラリを使用して簡略化**

### 6. 次の修正案

JoyStick.hのY軸を反転し、ボタン検出も参考に合わせる：

```cpp
// Y軸反転
int16_t getAxisY() const {
    return -convertAxis(_y, _centerY);  // マイナスで反転
}

// ボタン検出（参考では0以外が押下）
_button = (_wire->read() != 0);  // 0以外=押下
```
