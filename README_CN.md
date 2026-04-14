# DFRobot_IRPositionV2

* [English Version](./README.md)

**DFRobot_IRPositionV2** 是面向红外定位模组（**SKU：SEN0701**，传感器 **PAJ7025R2**）的 Arduino 主机库，通过 **CS32 I2C 桥接**固件访问。主机每次 `update()` 读取 **16 个对象 × 16 字节** 的完整帧，并可配置阈值、坐标缩放、曝光与最大跟踪目标数等参数。

## 目录

* [简介](#简介)
* [配置参数范围](#配置参数范围)
* [安装](#安装)
* [方法](#方法)
* [兼容性](#兼容性)
* [历史](#历史)
* [贡献者](#贡献者)

## 简介

* 每帧最多跟踪 **16** 个红外光点（索引 **0–15**）。
* 使用 **I2C** 通信，7 位从机地址为 **`0x58`**（Arduino 上常写成 8 位地址 **`0xB0`**）。`begin()` 内将 **`Wire`** 设为 **400 kHz**。
* 每个目标可读取 **中心坐标（12 位缩放后）**、原生 **98×98** 网格上的 **边界**、**平均/最大亮度**、**半径**（打包字段低 4 位）。
* 在读取任意目标数据前，请在循环中调用一次 **`update()`** 刷新内部帧缓存。

## 配置参数范围

在示例或产品集成中调节参数时，建议遵循下表范围（具体效果以实机与固件为准）：

| 项目 | 范围 / 说明 |
| ---- | ----------- |
| **I2C 地址** | 7 位 `0x58`（8 位常写作 `0xB0`）；`DFRobot_IRPositionV2(TwoWire*, uint8_t)` 传入 **8 位**地址。 |
| **`setIRBrightnessThreshold(brightness)`** | `brightness`：**0–255**（文档/固件常见默认 **0x97**）。数值越高，对亮度要求越严。 |
| **`setIRDetectionNoise(noise)`** | `noise`：**0–255**（常见默认 **0x0A**）。用于抑制噪声触发。 |
| **`setScaleResolution(x, y)`** | 每轴 **0–4095**（超过会被库内限制为 4095）。将传感器原生分辨率映射到 12 位输出；从 **98×98** 约放大 30 倍时常用 **2490×2490**。 |
| **`setMaxTrackingNumber(num)`** | `num`：**1–16**（库内会钳位）。 |
| **`setExposure(expo, settleMs)`** | `expo`：**0–65535**；`settleMs`：提交曝光后等待的毫秒数（默认 **10**），再读回更稳妥。 |
| **`getIRCoordinate(IRnum)`** | `IRnum`：**0–15**。返回打包的 **12 位 X/Y**，范围 **0–4095**（与 `setScaleResolution` 一致）。 |
| **`getIRBound(IRnum)`** | `IRnum`：**0–15**。返回 **上、下、左、右**，单位为原生网格 **0–98**。 |
| **`getAverageBrightness` / `getMaxBrightness`** | 返回值 **0–255**。 |
| **`getRadius(IRnum)`** | 半径为 **4 位** 字段（一般可理解为 **0–15** 像素量级）。 |

## 安装

1. 下载本库到 `\Arduino\libraries`（或通过库管理器安装，若已发布），然后打开 `examples` 目录运行示例。
2. 本库仅依赖 **Arduino** 核心与 **`Wire`**，无需其它第三方库。

## 方法

```c++
  /**
   * @fn DFRobot_IRPositionV2
   * @brief 默认构造函数（使用 `Wire` 与 7 位地址 `IRPOS_V2_I2C_ADDR` / 8 位 `0xB0`）。
   */
  DFRobot_IRPositionV2();

  /**
   * @fn DFRobot_IRPositionV2
   * @brief 指定 I2C 总线与 8 位地址（例如 `0xB0` → 7 位 `0x58`）。
   * @param pWire `TwoWire` 指针（NULL 则使用 `Wire`）。
   * @param I2C_addr 8 位 I2C 地址。
   */
  DFRobot_IRPositionV2(TwoWire *pWire, uint8_t I2C_addr);

  /**
   * @fn begin
   * @brief 初始化 I2C（400 kHz）并探测通信是否正常。
   * @return 探测读取成功返回 true，否则 false。
   */
  bool begin();

  /**
   * @fn update
   * @brief 读取一整帧（16 对象 × 16 字节）到内部缓存。
   * @return 成功返回 true，I2C 失败返回 false。
   */
  bool update();

  /**
   * @fn setIRBrightnessThreshold
   * @brief 设置红外光点亮度阈值（常见默认 0x97）。
   * @param brightness 0–255。
   * @return 成功返回 true，I2C 失败返回 false。
   */
  bool setIRBrightnessThreshold(uint8_t brightness);

  /**
   * @fn setIRDetectionNoise
   * @brief 设置红外检测噪声阈值（常见默认 0x0A）。
   * @param noise 0–255。
   * @return 成功返回 true，I2C 失败返回 false。
   */
  bool setIRDetectionNoise(uint8_t noise);

  /**
   * @fn setScaleResolution
   * @brief 设置 X/Y 坐标缩放（原生 98×98；每轴最大 4095）。
   * @param x_axis 0–4095。
   * @param y_axis 0–4095。
   * @return 成功返回 true，I2C 失败返回 false。
   */
  bool setScaleResolution(uint16_t x_axis, uint16_t y_axis);

  /**
   * @fn setMaxTrackingNumber
   * @brief 设置最大跟踪目标数（常见默认 0x10，即 16）。
   * @param num 1–16。
   * @return 成功返回 true，I2C 失败返回 false。
   */
  bool setMaxTrackingNumber(uint8_t num);

  /**
   * @fn setExposure
   * @brief 写入 16 位曝光、提交，并可在提交后延时再读回。
   * @param expo 0–65535。
   * @param settleMs 提交后延时（毫秒），默认 10。
   * @return 成功返回 true，I2C 失败返回 false。
   */
  bool setExposure(uint16_t expo, uint16_t settleMs = 10);

  /**
   * @fn getExposure
   * @brief 从读回寄存器读取当前曝光（16 位）。
   * @param expo 输出引用。
   * @return 成功返回 true，I2C 失败返回 false。
   */
  bool getExposure(uint16_t &expo);

  /**
   * @fn getIRCoordinate
   * @brief 获取上一帧 `update()` 后第 n 个光点的中心坐标。
   * @param IRnum 0–15。
   * @return 指向 `[xL, xH(低 4 位), yL, yH(低 4 位)]` 的指针（12 位坐标）。
   */
  uint8_t *getIRCoordinate(int IRnum);

  /**
   * @fn getIRBound
   * @brief 获取原生 0–98 网格上的边界。
   * @param IRnum 0–15。
   * @return 指向 `[Up, Down, Left, Right]` 的指针。
   */
  uint8_t *getIRBound(int IRnum);

  /**
   * @fn getAverageBrightness
   * @param IRnum 0–15。
   * @return 平均亮度 0–255；索引非法时为 0。
   */
  uint8_t getAverageBrightness(int IRnum);

  /**
   * @fn getMaxBrightness
   * @param IRnum 0–15。
   * @return 最大亮度 0–255；索引非法时为 0。
   */
  uint8_t getMaxBrightness(int IRnum);

  /**
   * @fn getRadius
   * @param IRnum 0–15。
   * @return 半径（打包字段低 4 位）；索引非法时为 0。
   */
  uint8_t getRadius(int IRnum);
```

## 兼容性

| 主板               | 表现良好 | 表现异常 | 未测试 | 备注 |
| ------------------ | :------: | :------: | :----: | ---- |
| Arduino Uno        |    √     |          |        |      |
| Arduino Leonardo   |    √     |          |        |      |
| Arduino MEGA2560   |    √     |          |        |      |
| FireBeetle-ESP32   |    √     |          |        |      |
| ESP8266            |    √     |          |        |      |
| FireBeetle-M0      |    √     |          |        |      |
| Micro:bit          |    √     |          |        |      |
| Raspberry Pi       |    √     |          |        |      |

## 历史

* Date 2026-04-13
* Version V1.0.0

## 贡献者

Written by thdyyl(yuanlong.yu@dfrobot.com), 2026-04-13 (Welcome to our [website](https://www.dfrobot.com/))
