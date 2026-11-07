# DFRobot_IRPositionV2

* [English Version](./README.md)

这款红外定位模组（SKU：SEN0701）采用 PixArt PAJ7025R2 红外图像传感芯片，配合板载 CS32 I2C 桥接控制器，实现多目标红外光点的实时检测与位置输出。模组每帧最多可同时跟踪 16 个红外光点，并给出每个光点的中心坐标、边界框、平均亮度、最大亮度和近似半径。传感芯片原生分辨率为约 98×98，桥接侧可将 X/Y 坐标按轴独立缩放到 12 位输出范围（0～4095）。主机通过 I2C 与模组通信，默认 7 位地址为 0x58，并可将新地址写入 Flash（有效范围 0x08～0x77，复位或重新上电后生效）。本库用于在 Arduino 等主控上驱动该模组：读取光点坐标与亮度，并设置亮度检测阈值、噪声抑制阈值、曝光时间、最大跟踪数量、指示灯和 I2C 地址；示例 `getPosition` 串口打印光点数据，`setI2CAddress` 保存新地址。树莓派 Python 版另提供终端与桌面窗口显示示例。

接线（I2C）：

- 红 → VCC（3.3V 或 5V，以模组规格为准）
- 黑 → GND
- 绿 → SCL
- 黄 → SDA

## 产品链接（https://www.dfrobot.com.cn）

SKU：SEN0701

## 目录

* [概述](#概述)
* [库安装](#库安装)
* [方法](#方法)
* [兼容性](#兼容性)
* [历史](#历史)
* [创作者](#创作者)

## 概述

本库用于读取红外光点，并设置检测阈值、坐标缩放、曝光、最大跟踪数量、指示灯和 I2C 地址。先调用 `begin()`，循环中调用一次 `update()` 后再读取各光点。示例 `getPosition` 打印坐标与亮度，示例 `setI2CAddress` 保存新的 7 位地址。

## 库安装

这里有 2 种安装方法：

1. 在 Arduino 库管理器中搜索并安装 DFRobot_IRPositionV2。
2. 下载库文件，解压到 Arduino 的 `libraries` 目录，然后打开 `examples` 目录运行示例。

本库仅依赖 Arduino 核心与 `Wire`，无需其它第三方库。

树莓派 / Python 版本位于 [`python/raspberry`](./python/raspberry/README_CN.md)，依赖 `smbus2`，接口为与 Arduino 版对应的 snake_case（示例 `get_position.py`、`set_i2c_address.py`）。

## 方法

```c++
  /**
   * @fn DFRobot_IRPositionV2
   * @brief 默认构造函数（使用 `Wire` 与 7 位地址 0x58）。
   */
  DFRobot_IRPositionV2();

  /**
   * @fn DFRobot_IRPositionV2
   * @brief 指定 I2C 总线与 7 位地址。
   * @param pWire `TwoWire` 指针（NULL 则使用 `Wire`）。
   * @param addr7bit 7 位 I2C 地址。
   */
  DFRobot_IRPositionV2(TwoWire *pWire, uint8_t addr7bit);

  /**
   * @fn begin
   * @brief 初始化 I2C（400 kHz）并探测通信是否正常。
   * @return 探测读取成功返回 true，否则 false。
   */
  bool begin();

  /**
   * @fn update
   * @brief 读取一整帧（16 对象 × 16 字节）到内部缓存。
   * @n 每圈循环调用一次，再使用 getIRCoordinate / getIRBound / 亮度 / 半径接口。
   * @return 成功返回 true，I2C 失败返回 false。
   */
  bool update();

  /**
   * @fn setIRBrightnessThreshold
   * @brief 设置红外光点亮度阈值。
   * @param brightness 阈值。
   * @n 范围 0–255，常见出厂默认 0x97。
   * @n 数值越高，亮度门槛越严，较暗的光点会被丢掉。
   * @return 成功返回 true，I2C 失败返回 false。
   */
  bool setIRBrightnessThreshold(uint8_t brightness);

  /**
   * @fn setIRDetectionNoise
   * @brief 设置红外检测噪声阈值。
   * @param noise 噪声阈值。
   * @n 范围 0–255，常见默认 0x0A，用于抑制噪声触发。
   * @return 成功返回 true，I2C 失败返回 false。
   */
  bool setIRDetectionNoise(uint8_t noise);

  /**
   * @fn setScaleResolution
   * @brief 设置 X/Y 坐标缩放。
   * @param x_axis X 轴缩放。
   * @param y_axis Y 轴缩放。
   * @n 每轴 0–4095，超过 4095 会被库限制为 4095。
   * @n 将传感器原生 98×98 网格映射到 12 位输出。示例使用 2490×2490。
   * @return 成功返回 true，I2C 失败返回 false。
   */
  bool setScaleResolution(uint16_t x_axis, uint16_t y_axis);

  /**
   * @fn setMaxTrackingNumber
   * @brief 设置最大跟踪目标数。
   * @param num 最大目标数。
   * @n 范围 1–16，库内会钳位。常见默认 0x10（16）。
   * @return 成功返回 true，I2C 失败返回 false。
   */
  bool setMaxTrackingNumber(uint8_t num);

  /**
   * @fn setExposure
   * @brief 写入 16 位曝光、提交，并固定等待 100 ms。
   * @param expo 曝光值。
   * @n 范围 100–65535。低于 100 时库内限制为 100。
   * @n 实际最大值还受帧周期限制。
   * @n 两次写入都成功后等待 100 ms；写入失败时不等待。
   * @return 两次写入都成功返回 true，I2C 失败返回 false。
   */
  bool setExposure(uint16_t expo);

  /**
   * @fn getExposure
   * @brief 从读回寄存器读取当前曝光（16 位）。
   * @param expo 输出引用。
   * @return 成功返回 true，I2C 失败返回 false。
   */
  bool getExposure(uint16_t &expo);

  /**
   * @fn setI2CAddress
   * @brief 将新的 7 位 I2C 地址保存到桥接芯片 flash。
   * @param addr7bit 新的 7 位地址。
   * @n 有效范围 0x08–0x77。
   * @n 仅在复位或重新上电后生效。
   * @n 地址非法、I2C 失败、flash 失败或超时（200 ms）时返回 false。
   * @return 保存成功返回 true，否则返回 false。
   */
  bool setI2CAddress(uint8_t addr7bit);

  /**
   * @fn getI2CAddress
   * @brief 读取当前生效的 7 位 I2C 地址。
   * @param addr7bit 输出引用。
   * @return 成功返回 true，I2C 失败返回 false。
   */
  bool getI2CAddress(uint8_t &addr7bit);

  /**
   * @fn setIndicator
   * @brief 打开或关闭板载指示灯。
   * @param status true 打开，false 关闭。
   * @return 成功返回 true，I2C 失败返回 false。
   */
  bool setIndicator(bool status);

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
   * @brief 上一帧 `update()` 中所选光点的平均亮度。
   * @param IRnum 0–15。
   * @n 返回 0–255；索引非法时返回 0。
   * @return 平均亮度。
   */
  uint8_t getAverageBrightness(int IRnum);

  /**
   * @fn getMaxBrightness
   * @brief 上一帧 `update()` 中所选光点的最大亮度。
   * @param IRnum 0–15。
   * @n 返回 0–255；索引非法时返回 0。
   * @return 最大亮度。
   */
  uint8_t getMaxBrightness(int IRnum);

  /**
   * @fn getRadius
   * @brief 上一帧 `update()` 中所选光点的半径。
   * @param IRnum 0–15。
   * @n 打包字段的低 4 位，一般为 0–15。索引非法时返回 0。
   * @return 半径。
   */
  uint8_t getRadius(int IRnum);
```

## 兼容性

| 主板               | 通过 | 未通过 | 未测试 | 备注 |
| ------------------ | :--: | :----: | :----: | ---- |
| Arduino Uno        |  √   |        |        |      |
| Arduino Leonardo   |  √   |        |        |      |
| Arduino MEGA2560   |  √   |        |        |      |
| FireBeetle-ESP32   |  √   |        |        |      |
| ESP8266            |  √   |        |        |      |
| FireBeetle-M0      |  √   |        |        |      |
| Micro:bit          |  √   |        |        |      |
| Raspberry Pi       |  √   |        |        |      |

## 历史

* Date 2026-11-07
* Version V1.0.0

## 创作者

Written by thdyyl(yuanlong.yu@dfrobot.com), 2026-11-07. (Welcome to our [website](https://www.dfrobot.com/))
