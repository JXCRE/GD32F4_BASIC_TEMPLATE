# GD32F4基础模板   

基于GD32F427VG实现   
该驱动库针对于裸机初始化使用

## 工程文件架构   

    ├─1_Keilprj             keil工程文件
    ├─2_Source              
    │  ├─Board
    │  │  ├─CMSIS
    │  │  ├─Library         标准库函数
    │  │  │  ├─Include
    │  │  │  └─Source
    │  │  └─Startup         启动文件
    │  ├─Bsp                统一接口初始化源码
    │  ├─Common             工程通用的工具函数实现源码
    │  ├─Lib                库文件
    │  └─User               用户自定义业务、私有功能实现源码
    ├─3_Output              编译产生的中间件，hex，axf等文件
    ├─4_Doc                 工程相关的文档
    └─5_Burn                axf文件生成的bin文件(应该或许Maybe有人能用到)

## 常用接口   

*已实现通用驱动*   

- gpio
- exit

*未实现通用驱动*   

- adc
- dac
- dma
- fmc
- i2c
- spi
- timer
- uart

## 适配同系列其他芯片   

## 版本详情   


