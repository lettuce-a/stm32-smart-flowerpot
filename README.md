# STM32 智能花盆系统

基于 **STM32F103C8T6** 的智能花盆控制工程。主控读取环境、土壤、光照和空气质量数据，在 OLED 显示；通过 ESP-01S 将数据发送到电脑端，并可自动控制补光灯和水泵。

> 本仓库只包含嵌入式工程，**不包含网页仪表盘**、编译产物和任何 Wi-Fi 密码。

## 已实现功能

- DHT11：温度、湿度采集与 OLED 显示
- YL-69：土壤湿度采集、滤波、湿度百分比换算
- SGP30：eCO₂ 与 TVOC 采集（I²C，CRC 校验）
- 光敏电阻：ADC 光照采集，补光灯自动/强制开/强制关
- MOSFET-15A 水泵：自动浇水与 3 秒手动浇水
- ESP-01S：连接 Wi-Fi，通过 TCP 向电脑端发送传感器数据
- OLED：开机加载动画、四页数据页面
- 按键：翻页、自动/手动浇水、土壤阈值切换、灯光模式切换
- 蜂鸣器：按键确认与土壤过干提示

## 接线总览

| 模块 / 功能 | STM32F103C8T6 引脚 | 说明 |
|---|---:|---|
| OLED、SGP30 | PB6 / PB7 | I²C1：SCL / SDA |
| DHT11 DATA | PA11 | 单总线数字输入 |
| 土壤湿度 AO | PA1 | ADC1_IN1 |
| 光敏模块 AO | PA0 | ADC1_IN0 |
| ESP-01S | PA2 / PA3 | USART2：STM32 TX / RX |
| 补光灯 | PB0 | TIM3_CH3 PWM |
| 水泵 MOSFET 输入 | PB1 | 高电平开启 |
| 蜂鸣器 | PB5 | TIM3_CH2，约 2.4 kHz |
| KEY1 | PB12 | OLED 翻页 |
| KEY2 | PB13 | 自动/手动浇水模式；手动时浇水 3 秒 |
| KEY3 | PB14 | 自动浇水阈值切换：25% / 30% / 35% |
| KEY4 | PB15 | 补光灯：自动 / 强制开 / 强制关 |

## 当前控制策略

- 土壤湿度低于自动阈值时，自动浇水 3 秒；阈值可由 KEY3 在 25%、30%、35% 间切换。
- 土壤湿度达到恢复阈值后停止自动浇水，避免反复启停。
- 08:00–22:00 的补光时段内，光照 ADC 高于 1800 时自动补光；低于 1000 时关闭。双阈值和确认延时用于防闪烁。
- 光敏模块应避免被补光灯直接照射，否则会造成光照反馈导致频繁开关。

## 配置 Wi-Fi 与电脑接收地址

工程中的 `USER/wifi_config.h` 不上传 GitHub。第一次下载源码后：

1. 将 `USER/wifi_config.h.example` 复制并重命名为 `USER/wifi_config.h`。
2. 填写你的 Wi-Fi 名称、密码、电脑 IPv4 地址和接收端口。
3. 在 Packet Sender 中启动 TCP Server，端口需与 `ESP_SERVER_PORT` 一致（当前示例为 `8080`）。

发送的数据格式示例：

```text
T=29,H=59,SOIL=43,ADC=2840,CO2=956,TVOC=159
```

## 编译与烧录

1. 使用 Keil MDK 打开 `USER/OLED.uvprojx`。
2. 选择 `Project → Rebuild all target files`。
3. 使用 ST-Link 连接 SWD 接口后，选择 `Flash → Download`。
4. 若出现 `wifi_config.h: No such file`，按上节创建本机 Wi-Fi 配置文件。

## 目录说明

- `USER/`：主程序、Keil 工程文件与中断配置
- `HARDWARE/OLED/`：OLED 驱动
- `HAL/`、`CMSIS/`、`CORE/`、`SYSTEM/`、`STM32F10x_FWLib/`：STM32 库与底层支持文件
- `OBJ/`、`Listings/`：Keil 自动生成，已排除

## 许可证

本项目用于课程设计和学习。若要公开复用，请保留来源说明，并先按你的硬件与网络环境进行测试。
