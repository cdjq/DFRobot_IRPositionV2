DFRobot_IRPositionV2
===========================

* [English Version](./README.md)

这款红外定位模组（SKU：SEN0701）采用 PixArt PAJ7025R2 红外图像传感芯片，配合板载 CS32 I2C 桥接控制器，实现多目标红外光点的实时检测与位置输出。模组每帧最多可同时跟踪 16 个红外光点，并给出每个光点的中心坐标、边界框、平均亮度、最大亮度和近似半径。传感芯片原生分辨率为约 98×98，桥接侧可将 X/Y 坐标按轴独立缩放到 12 位输出范围（0～4095）。主机通过 I2C 与模组通信，默认 7 位地址为 0x58，并可将新地址写入 Flash（有效范围 0x08～0x77，复位或重新上电后生效）。本库是树莓派上的 Python 驱动：读取光点坐标与亮度，并设置亮度检测阈值、噪声抑制阈值、曝光时间、最大跟踪数量、指示灯和 I2C 地址。单帧数据为 256 字节，寄存器地址为 16 位，因此依赖 `smbus2`。示例 `get_position.py` 可在终端或桌面窗口显示光点，`set_i2c_address.py` 保存新地址。

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

本库在树莓派上读取红外光点，并设置检测阈值、坐标缩放、曝光、最大跟踪数量、指示灯和 I2C 地址。读取各光点前先调用一次 `update()`。示例 `get_position.py` 可在终端或桌面窗口显示光点，`set_i2c_address.py` 保存新的 7 位地址。

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

桌面窗口模式需要 `python3-tk`（`sudo apt install python3-tk`）。没有桌面或未安装 tkinter 时，`get_position.py` 会自动改用终端输出。

## 方法

```python

def __init__(self, bus=1, addr=0x58):
  '''!
    @brief DFRobot_IRPositionV2 构造函数
    @param bus  树莓派 I2C 总线号，通常为 1（/dev/i2c-1）
    @param addr 桥接芯片的 7 位 I2C 地址，默认 0x58
  '''

def begin(self):
  '''!
    @brief 初始化红外定位模组，并通过读取帧区域探测通信是否正常
    @return 返回初始化状态
    @retval True  成功
    @retval False 失败，I2C 错误
  '''

def update(self):
  '''!
    @brief 读取一整帧（16 对象 × 16 字节）到内部缓存
    @n 每圈循环调用一次，再使用 get_ir_coordinate / get_ir_bound / 亮度 / 半径接口
    @return 成功返回 True，I2C 失败返回 False
  '''

def set_ir_brightness_threshold(self, brightness):
  '''!
    @brief 设置红外光点亮度阈值
    @param brightness 阈值
    @n 范围 0–255，常见出厂默认 0x97
    @n 数值越高，亮度门槛越严，较暗的光点会被丢掉
    @return 成功返回 True，I2C 失败返回 False
  '''

def set_ir_detection_noise(self, noise):
  '''!
    @brief 设置红外检测噪声阈值
    @param noise 噪声阈值
    @n 范围 0–255，常见默认 0x0A，用于抑制噪声触发
    @return 成功返回 True，I2C 失败返回 False
  '''

def set_scale_resolution(self, x_axis, y_axis):
  '''!
    @brief 设置 X/Y 坐标缩放
    @param x_axis X 轴缩放
    @param y_axis Y 轴缩放
    @n 每轴 0–4095，超过 4095 会被库限制为 4095
    @n 将传感器原生 98×98 网格映射到 12 位输出。示例使用 2490×2490
    @return 成功返回 True，I2C 失败返回 False
  '''

def set_max_tracking_number(self, num):
  '''!
    @brief 设置最大跟踪目标数
    @param num 最大目标数
    @n 范围 1–16，库内会钳位。常见默认 0x10（16）
    @return 成功返回 True，I2C 失败返回 False
  '''

def set_exposure(self, expo):
  '''!
    @brief 写入 16 位曝光、提交，并固定等待 100 ms
    @param expo 曝光值
    @n 范围 100–65535。低于 100 时库内限制为 100
    @n 实际最大值还受帧周期限制
    @n 两次写入都成功后等待 100 ms；写入失败时不等待
    @return 两次写入都成功返回 True，I2C 失败返回 False
  '''

def get_exposure(self):
  '''!
    @brief 从读回寄存器读取当前曝光（16 位）
    @return 返回曝光值 100–65535，I2C 失败返回 -1
  '''

def set_i2c_address(self, addr7bit):
  '''!
    @brief 将新的 7 位 I2C 地址保存到桥接芯片 flash
    @param addr7bit 新的 7 位地址
    @n 有效范围 0x08–0x77
    @n 仅在复位或重新上电后生效
    @n 地址非法、I2C 失败、flash 失败或超时（200 ms）时返回 False
    @return 保存成功返回 True，否则返回 False
  '''

def get_i2c_address(self):
  '''!
    @brief 读取当前生效的 7 位 I2C 地址
    @return 返回当前生效的 7 位地址，I2C 失败返回 -1
  '''

def set_indicator(self, status):
  '''!
    @brief 打开/关闭传感器上的指示灯
    @param status True 打开，False 关闭
    @return 设置成功返回 True，I2C 失败返回 False
  '''

def get_ir_coordinate(self, ir_num):
  '''!
    @brief 获取上一帧 update() 后第 n 个光点的中心坐标
    @param ir_num 索引 0–15
    @return 返回列表 [x, y]，每个为 12 位（缩放后 0–4095）；索引非法时返回 [0, 0]
  '''

def get_ir_bound(self, ir_num):
  '''!
    @brief 获取光点在原生 0–98 网格上的边界
    @param ir_num 索引 0–15
    @return 返回列表 [up, down, left, right]；索引非法时返回 [0, 0, 0, 0]
  '''

def get_average_brightness(self, ir_num):
  '''!
    @brief 上一帧 update() 中所选光点的平均亮度
    @param ir_num 索引 0–15
    @n 返回 0–255；索引非法时返回 0
    @return 平均亮度
  '''

def get_max_brightness(self, ir_num):
  '''!
    @brief 上一帧 update() 中所选光点的最大亮度
    @param ir_num 索引 0–15
    @n 返回 0–255；索引非法时返回 0
    @return 最大亮度
  '''

def get_radius(self, ir_num):
  '''!
    @brief 上一帧 update() 中所选光点的半径
    @param ir_num 索引 0–15
    @n 打包字段的低 4 位，一般为 0–15。索引非法时返回 0
    @return 半径
  '''
```

## 兼容性

* RaspberryPi Version

| 主板         | 通过 | 未通过 | 未测试 | 备注 |
| ------------ | :-------: | :--------: | :------: | ------- |
| RaspberryPi2 |          |            |    √    |         |
| RaspberryPi3 |          |            |    √    |         |
| RaspberryPi4 |    √    |            |          |         |

* Python Version

| Python  | 通过 | 未通过 | 未测试 | 备注 |
| ------- | :-------: | :--------: | :------: | ------- |
| Python2 |          |            |    √    |         |
| Python3 |    √    |            |          |         |

## 历史

- 2026-11-07 - 1.0.0 版本

## 创作者

Written by thdyyl(yuanlong.yu@dfrobot.com), 2026-11-07. (Welcome to our [website](https://www.dfrobot.com/))
