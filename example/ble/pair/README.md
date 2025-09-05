# BLE pair示例

源码路径：example/ble/pair

(Platform_peri)=
## 支持的平台
<!-- 支持哪些板子和芯片平台 -->
全平台

## 概述
<!-- 例程简介 -->
本例程演示了本平台如何做GAP peripheral进行配对。


## 例程的使用
<!-- 说明如何使用例程，比如连接哪些硬件管脚观察波形，编译和烧写可以引用相关文档。
对于rt_device的例程，还需要把本例程用到的配置开关列出来，比如PWM例程用到了PWM1，需要在onchip菜单里使能PWM1 -->
1. 作为从设备时开机会开启广播，广播名字以SIFLI_APP-xx-xx-xx-xx-xx-xx, 其中xx代表本设备的蓝牙地址。可以通过手机的BLE APP进行连接
2. 作为GATT server时，可以在手机端进行write和read操作，或者使能CCCD，设备会每一秒更新一次特征值。
3. 在初始化阶段，默认设置bond_ack为BOND_PENDING，需要关注CONNECTION_MANAGER_BOND_AUTH_INFOR并适时调用connection_manager_bond_ack_reply。
```c
connection_manager_set_bond_ack(BOND_PENDING);
```

4. 默认设置IO为GAP_IO_CAP_DISPLAY_ONLY，也可以通过finsh命令cmd_diss set_io iocap来重设IO。
```c
connection_manager_set_bond_cnf_iocap(GAP_IO_CAP_DISPLAY_ONLY);
```

5. 当设置为INPUT_ONLY的时候，需要通过finsh命令cmd_diss set_key conn_idx key输入key

6. 通过finsh命令cmd_diss set_key conn_idx sec_level可以要求central发起配对


### 硬件需求
运行该例程前，需要准备：
+ 一块本例程支持的开发板([支持的平台](#Platform_peri))。
+ 手机设备。

### menuconfig配置
1. 使能蓝牙(`BLUETOOTH`)：\
![BLUETOOTH](./assets/bluetooth.png)
2. 使能GAP, GATT Client, BLE connection manager：\
![BLE MIX](./assets/gap_gatt_ble_cm.png)
3. 使能NVDS：\
![NVDS](./assets/bt_nvds.png)


### 编译和烧录
切换到例程project目录，运行scons命令执行编译：
```c
> scons --board=eh-lb525 -j32
```
切换到例程`project/build_xx`目录，运行`uart_download.bat`，按提示选择端口即可进行下载：
```c
$ ./uart_download.bat

     Uart Download

please input the serial port num:5
```
关于编译、下载的详细步骤，请参考[快速入门](/quickstart/get-started.md)的相关介绍。

## 例程的预期结果
<!-- 说明例程运行结果，比如哪几个灯会亮，会打印哪些log，以便用户判断例程是否正常运行，运行结果可以结合代码分步骤说明 -->
例程启动后：
1. 可以被手机BLE APP搜到并连接，进行相应的GATT特质值read/write等操作。
2. 通过修改io，然后在手机端发起配对，可以实现不同的配对鉴权方式。
3. 也可以通过finsh命令从peripheral开发板侧发起加密请求然后配对。

## 异常诊断


## 参考文档
<!-- 对于rt_device的示例，rt-thread官网文档提供的较详细说明，可以在这里添加网页链接，例如，参考RT-Thread的[RTC文档](https://www.rt-thread.org/document/site/#/rt-thread-version/rt-thread-standard/programming-manual/device/rtc/rtc) -->

## 更新记录
|版本 |日期   |发布说明 |
|:---|:---|:---|
|0.0.1 |01/2025 |初始版本 |
| | | |
| | | |
