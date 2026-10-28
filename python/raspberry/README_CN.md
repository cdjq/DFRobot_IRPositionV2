DFRobot_IRPositionV2
===========================

* [English Version](./README.md)

红外定位模组（SKU：SEN0701）每帧最多跟踪 16 个红外光点，并给出每个光点的中心坐标、边界、亮度和半径。模组通过 I2C 连接，默认 7 位地址为 0x58。

本库用于在树莓派上驱动该模组。调用 `update()` 读取一帧后，可设置检测阈值、坐标缩放、曝光、最大跟踪数量、指示灯和模组 I2C 地址。单帧为 256 字节、寄存器地址为 16 位，因此依赖 `smbus2`。

## 产品链接（https://www.dfrobot.com）

    SKU：SEN0701

## 目录

* [概述](#概述)
* [配置参数范围](#配置参数范围)
* [库安装](#库安装)
* [示例](#示例)
* [方法](#方法)
* [兼容性](#兼容性)
* [历史](#历史)
* [创作者](#创作者)

## 概述

* 每帧最多跟踪 **16** 个红外光点（索引 **0–15**）。
* 使用 **I2C** 通信，7 位从机地址为 **`0x58`**。桥接芯片使用 **16 位寄存器地址**，单帧数据 **256 字节**，因此本库依赖 **`smbus2`**（`i2c_rdwr` 重复起始读取）。
* 每个目标可读取 **中心坐标（12 位缩放后）**、原生 **98×98** 网格上的 **边界**、**平均/最大亮度**、**半径**（打包字段低 4 位）。
* 在读取任意目标数据前，请在循环中调用一次 **`update()`** 刷新内部帧缓存。

## 配置参数范围

在示例或产品集成中调节参数时，建议遵循下表范围（具体效果以实机与固件为准）：

| 项目 | 范围 / 说明 |
| ---- | ----------- |
| **I2C 地址** | 7 位 `0x58`；`DFRobot_IRPositionV2(bus, addr)` 传入总线号与 **7 位**地址。 |
| **`set_i2c_address(addr7bit)`** | `addr7bit`：**0x08–0x77**。保存到桥接芯片 flash，复位或重新上电后生效。保存完成即返回；地址非法、通信失败、flash 保存失败，或内部时限（200 ms）内仍未完成时返回 `False`。 |
| **`set_ir_brightness_threshold(brightness)`** | `brightness`：**0–255**（文档/固件常见默认 **0x97**）。数值越高，对亮度要求越严。 |
| **`set_ir_detection_noise(noise)`** | `noise`：**0–255**（常见默认 **0x0A**）。用于抑制噪声触发。 |
| **`set_scale_resolution(x, y)`** | 每轴 **0–4095**（超过会被库内限制为 4095）。将传感器原生分辨率映射到 12 位输出；从 **98×98** 约放大 30 倍时常用 **2490×2490**。 |
| **`set_max_tracking_number(num)`** | `num`：**1–16**（库内会钳位）。 |
| **`set_exposure(expo)`** | `expo`：**100–65535**（低于 100 时库内会限制为 100，实际最大值还受帧周期限制）。两次写入都成功即返回 `True`，随后固定等待 **100 ms**。写入失败时不等待。需要当前值时调用 `get_exposure()`。 |
| **`get_ir_coordinate(ir_num)`** | `ir_num`：**0–15**。返回 `[x, y]`，每个为 **12 位**，范围 **0–4095**（与 `set_scale_resolution` 一致）。 |
| **`get_ir_bound(ir_num)`** | `ir_num`：**0–15**。返回 `[up, down, left, right]`（上、下、左、右），单位为原生网格 **0–98**。 |
| **`get_average_brightness` / `get_max_brightness`** | 返回值 **0–255**。 |
| **`get_radius(ir_num)`** | 半径为 **4 位** 字段（一般可理解为 **0–15** 像素量级）。 |

## 库安装

1. 在树莓派上开启 I2C 接口（`sudo raspi-config` → Interface Options → I2C）。如需更高帧率，可在 `/boot/config.txt` 中加入 `dtparam=i2c_arm_baudrate=400000` 将总线设为 400 kHz，然后重启。
2. 安装依赖 `smbus2`：

```shell
sudo pip install smbus2
```

3. 将库下载到树莓派，然后打开 `examples` 目录。要执行一个例程 demox.py，请在命令行中输入 python demox.py。例如，要执行 get_position.py 例程，你需要输入：

```python
python get_position.py
```

## 示例

* `get_position.py`：读取红外目标坐标与亮度等数据。脚本顶部的 `DISPLAY_MODE` 用于选择输出方式：
  * `DISPLAY_CONSOLE`：每帧在终端打印全部 16 个槽位，适用于无桌面的 Raspberry Pi OS Lite。
  * `DISPLAY_GUI`（默认）：tkinter 窗口，在二维画布上实时绘制被跟踪的光点（编号、半径、边界框），列出坐标与亮度，并提供亮度阈值 / 噪声阈值 / 曝光滑块和指示灯开关。需要树莓派桌面环境或 `ssh -X`；若缺少 tkinter 可执行 `sudo apt install python3-tk`。当 tkinter 或显示设备不可用时，脚本会自动退回控制台模式。
* `set_i2c_address.py`：保存新的 7 位桥接地址到 flash，成功后需要复位或重新上电。

## 方法

```python
  '''!
    @brief DFRobot_IRPositionV2 构造函数
    @param bus  树莓派 I2C 总线号，通常为 1（/dev/i2c-1）
    @param addr 桥接芯片的 7 位 I2C 地址，默认 0x58
  '''
  def __init__(self, bus=1, addr=0x58):

  '''!
    @brief 初始化红外定位模组，并通过读取帧区域探测通信是否正常
    @return 返回初始化状态
    @retval True  成功
    @retval False 失败，I2C 错误
  '''
  def begin(self):

  '''!
    @brief 读取一整帧（16 对象 × 16 字节）到内部缓存
    @details 每次循环调用一次，之后再调用 get_ir_coordinate() 等访问函数，以减少 I2C 通信量
    @return 返回更新状态
    @retval True  成功
    @retval False 失败，I2C 错误
  '''
  def update(self):

  '''!
    @brief 设置红外光点亮度阈值（固件常见默认 0x97）
    @param brightness 范围 0–255，数值越高对亮度要求越严
    @return 成功返回 True，I2C 失败返回 False
  '''
  def set_ir_brightness_threshold(self, brightness):

  '''!
    @brief 设置红外检测噪声阈值（常见默认 0x0A）
    @param noise 范围 0–255
    @return 成功返回 True，I2C 失败返回 False
  '''
  def set_ir_detection_noise(self, noise):

  '''!
    @brief 设置 X/Y 坐标缩放（原生 98×98；每轴最大 4095）
    @param x_axis 范围 0–4095，超过会被限制为 4095
    @param y_axis 范围 0–4095，超过会被限制为 4095
    @return 成功返回 True，I2C 失败返回 False
  '''
  def set_scale_resolution(self, x_axis, y_axis):

  '''!
    @brief 设置最大跟踪目标数（常见默认 0x10，即 16）
    @param num 范围 1–16，库内会钳位
    @return 成功返回 True，I2C 失败返回 False
  '''
  def set_max_tracking_number(self, num):

  '''!
    @brief 写入 16 位曝光、提交，并固定等待 100 ms
    @param expo 范围 100–65535。低于 100 时库内会限制为 100，实际最大值还受帧周期限制
    @return 两次写入都成功返回 True，I2C 失败返回 False。写入失败时不等待。
  '''
  def set_exposure(self, expo):

  '''!
    @brief 从读回寄存器读取当前曝光（16 位）
    @return 返回曝光值 100–65535，I2C 失败返回 -1
  '''
  def get_exposure(self):

  '''!
    @brief 保存新的 7 位 I2C 地址到桥接芯片 flash，复位或重新上电后生效
    @param addr7bit 新 7 位地址，范围 0x08–0x77
    @return 保存成功返回 True；地址非法、I2C 失败、flash 失败或超时返回 False
  '''
  def set_i2c_address(self, addr7bit):

  '''!
    @brief 读取当前生效的 7 位 I2C 地址
    @return 返回当前生效的 7 位地址，I2C 失败返回 -1
  '''
  def get_i2c_address(self):

  '''!
    @brief 打开/关闭传感器上的指示灯
    @param status True 打开，False 关闭
    @return 设置成功返回 True，I2C 失败返回 False
  '''
  def set_indicator(self, status):

  '''!
    @brief 获取上一帧 update() 后第 n 个光点的中心坐标
    @param ir_num 索引 0–15
    @return 返回列表 [x, y]，每个为 12 位（缩放后 0–4095）；索引非法时返回 [0, 0]
  '''
  def get_ir_coordinate(self, ir_num):

  '''!
    @brief 获取光点在原生 0–98 网格上的边界
    @param ir_num 索引 0–15
    @return 返回列表 [up, down, left, right]；索引非法时返回 [0, 0, 0, 0]
  '''
  def get_ir_bound(self, ir_num):

  '''!
    @brief 获取上一帧中所选光点的平均亮度
    @param ir_num 索引 0–15
    @return 返回亮度 0–255；索引非法时为 0
  '''
  def get_average_brightness(self, ir_num):

  '''!
    @brief 获取上一帧中所选光点的最大亮度
    @param ir_num 索引 0–15
    @return 返回亮度 0–255；索引非法时为 0
  '''
  def get_max_brightness(self, ir_num):

  '''!
    @brief 获取上一帧中所选光点的半径（像素）
    @param ir_num 索引 0–15
    @return 返回半径（打包字段低 4 位）；索引非法时为 0
  '''
  def get_radius(self, ir_num):
```

## 兼容性

* RaspberryPi Version

| Board        | Work Well | Work Wrong | Untested | Remarks |
| ------------ | :-------: | :--------: | :------: | ------- |
| RaspberryPi2 |          |            |    √    |         |
| RaspberryPi3 |          |            |    √    |         |
| RaspberryPi4 |    √    |            |          |         |

* Python Version

| Python  | Work Well | Work Wrong | Untested | Remarks |
| ------- | :-------: | :--------: | :------: | ------- |
| Python2 |          |            |    √    |         |
| Python3 |    √    |            |          |         |

## 历史

- 2026-10-26 - 1.0.0 版本

## 创作者

Written by thdyyl(yuanlong.yu@dfrobot.com), 2026. (Welcome to our [website](https://www.dfrobot.com/))
