本科生毕业设计

1、实物全貌
<img width="640" height="280" alt="1fd25df19b6b8ad09184025a7915dced_720" src="https://github.com/user-attachments/assets/3037b680-4c8d-4b73-9f01-fb91fe236482" />

2、效果演示
<img width="640" height="280" alt="a77666019f4f0a6c5c426bfff2c7901d" src="https://github.com/user-attachments/assets/bd8487a1-f221-4f7e-97a2-d7ca37984ef5" />


<img width="300" height="500" alt="229362d045f348fcddcb113ad39322e1_720" src="https://github.com/user-attachments/assets/ceef0f31-0543-481c-bec7-96fcfaca8241" />


3、介绍

3.1 STM32MP157
在开发板拷贝onnxruntime库，烧录exe程序执行yolo推理，识别PCB瑕疵。通过uart让stm32f103c8t6控制舵机。

3.2 STM32F103C8T6
烧录hex程序，控制舵机、电机，通过mqtt（esp8266模块）连接onenet物联网平台上传瑕疵种类、数量。

3.3 微信小程序
获取onenet平台API，显示瑕疵，以及控制电机。

