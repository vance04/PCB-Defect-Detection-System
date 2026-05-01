本科生毕业设计

1、实物全貌
<img width="640" height="280" alt="1fd25df19b6b8ad09184025a7915dced_720" src="https://github.com/user-attachments/assets/3037b680-4c8d-4b73-9f01-fb91fe236482" />

2、效果演示
<img width="640" height="280" alt="1b24f9aa6789b0994d302aa4dd5fd379_720" src="https://github.com/user-attachments/assets/21513043-ab96-464a-bcbe-b2100a35c93b" />

<img width="220" height="470" alt="image" src="https://github.com/user-attachments/assets/9a78faba-cbdd-4811-ab9a-023127c94c20" />

3、介绍

3.1 STM32MP157
在开发板拷贝onnxruntime库，烧录exe程序执行yolo推理，识别PCB瑕疵。通过uart让stm32f103c8t6控制舵机。

3.2 STM32F103C8T6
烧录hex程序，控制舵机、电机，通过mqtt（esp8266模块）连接onenet物联网平台上传瑕疵种类、数量。

3.3 微信小程序
获取onenet平台API，显示瑕疵，以及控制电机。

