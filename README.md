# 倒立摆 PID 控制(STM32F103)

基于 STM32F103C8T6 的旋转倒立摆控制系统,实现**自动起摆**与**串级 PID 平衡控制**,并通过 OLED 和串口实时输出运行状态。

## 功能特性

- **自动起摆**:检测摆杆摆动的局部极值判断运动方向,输出正/反向 PWM 脉冲串,逐步把摆杆甩到竖直位置
- **串级 PID**:角度环(内环)+ 位置环(外环),位置环输出经一阶低通后叠加到角度环目标上
- **角度反馈**:12 位 ADC 读取电位器,8 次采样取平均抑制噪声
- **速度/位置反馈**:TIM3 编码器接口测速,速度积分得到小车位置
- **人机交互**:按键启停、左右移动目标位置;OLED 实时显示两环参数、目标值、实际值和输出
- **数据上传**:串口按 CSV 格式输出角度环数据,可直接用上位机绘图调参

## 硬件接口

| 功能 | 外设 / 引脚 |
| --- | --- |
| 角度传感器 | ADC1_IN8 / PB0 |
| 电机 PWM | TIM2_CH1 / PA0 |
| 电机方向 | PB12、PB13 |
| 编码器 | TIM3_CH1/CH2 / PA6、PA7(编码器模式 TI12) |
| 调参电位器 | ADC2_CH2~CH5 / PA2~PA5(代码中预留,用于调试) |
| OLED | 软件 I2C,SCL = PB8,SDA = PB9 |
| 串口 | USART1 / PA9(TX)、PA10(RX),9600 8N1 |

## 控制结构

控制任务全部运行在 TIM1 更新中断中:

| 任务 | 周期 | 频率 |
| --- | --- | --- |
| 定时器中断本体 | 1 ms | 1 kHz |
| 按键扫描 | 1 ms | 1 kHz |
| 角度采样与速度积分 | 1 ms | 1 kHz |
| 角度环 PID | 5 ms | 200 Hz |
| 位置环 PID | 50 ms | 20 Hz |
| 起摆方向检测 | 40 ms | 25 Hz |

- 定时器:TIM1 时钟 72 MHz,预分频 720,周期 100 → **1 kHz** 更新中断
- 角度环输出取反后驱动电机(`Motor_SetPWM(-AnglePID.Out)`),用于匹配电机与摆杆的安装方向
- 位置环输出经一阶低通滤波后修正角度环目标:

```c
loc_out_f = 0.3f * locationPID.Out + 0.7f * loc_out_f;
AnglePID.Target = CENTER_ANGLE - loc_out_f;
```

### 自动起摆状态机

| RunState | 含义 |
| --- | --- |
| `0` | 停机,电机输出 0 |
| `1` | 起摆检测:每 40 ms 采样一次角度,通过三点极值判断摆动方向 |
| `21 → 22 → 23 → 24` | 正向起摆脉冲:先正转 100 ms,再反转 100 ms |
| `31 → 32 → 33 → 34` | 反向起摆脉冲:先反转 100 ms,再正转 100 ms |
| `4` | 平衡控制:角度环 + 位置环工作 |

进入 `4` 状态时会把 `location` 和角度环积分项清零,避免起摆过程中的累积量影响平衡。

### PID 实现

位置式 PID,输出限幅 ±100:

```c
error0 = Target - Actual;
errorInt += error0;                  // Ki 为 0 时积分项清零
Out = Kp*error0 + Ki*errorInt + Kd*(error0 - error1);
```

## 默认参数

| 参数 | 值 | 说明 |
| --- | --- | --- |
| `CENTER_ANGLE` | 2083 | 摆杆竖直时的 ADC 值 |
| `CENTER_RANGE` | 500 | 判定"接近竖直"的角度范围 |
| `START_PWM` | 35 | 起摆脉冲幅值 |
| `START_TIME` | 100 | 起摆脉冲持续时间(ms) |
| 角度环 Kp / Ki / Kd | 0.3 / 0.02 / 0.66 | 内环 |
| 位置环 Kp / Ki / Kd | 0.4 / 0 / 4 | 外环 |

## 目录结构

```
.
├── Project.uvprojx      # Keil MDK 工程文件
├── User/                # 应用层
│   ├── main.c           # 主循环、状态机、PID 调度
│   ├── PID.c / PID.h    # PID 算法
│   └── stm32f10x_it.c   # 中断服务
├── HardWare/            # 外设驱动
│   ├── AD.c             # 角度采集(ADC1)
│   ├── Encoder.c        # 编码器测速(TIM3)
│   ├── Motor.c          # 电机方向 + PWM 输出
│   ├── PWM.c            # TIM2 PWM
│   ├── OLED.c/.h        # OLED 显示(软件 I2C)
│   ├── Key.c            # 按键
│   ├── LED.c            # 运行指示灯
│   ├── Serial.c         # 串口
│   └── RP.c             # 调参电位器(预留)
├── System/              # Delay、Timer
├── Start/               # 启动文件与 CMSIS
├── Library/             # STM32F10x 标准外设库
└── keilkill.bat         # 清理 Keil 编译产物
```

## 编译与下载

1. 安装 Keil MDK 5 和 STM32F1xx Device Family Pack
2. 打开 `Project.uvprojx`
3. `F7` 编译,`F8` 下载(需 ST-Link / J-Link)

编译产物(`Objects/`、`Listings/`、`DebugConfig/`)已通过 `.gitignore` 排除。

## 串口输出

主循环中按 CSV 格式持续输出:

```
角度环目标, 当前角度, 角度环输出, 角度环积分项
```

## 说明

- 源码中的中文注释存在编码混用:`User/main.c`、`User/PID.c`、`User/PID.h` 为 **GBK**,`HardWare/AD.c` 等为 **UTF-8**。在 GitHub 网页直接查看 GBK 文件时中文可能显示为乱码,用 Keil 或设置编码后的编辑器打开正常。
- `User/PID.c.bak` 为调试留下的备份文件,已被 `.gitignore` 排除。
