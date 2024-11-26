# ESP32-Bluetooth-Page-Turner


Finding the right solution for integrating a Bluetooth remote with the ESP32 was a real challenge. After countless trials, errors, and digging through fragmented resources, I finally managed to make it work. To save others from the same frustration, I’m making this project open-source.

Feel free to use, adapt, and improve it—let’s make these kinds of integrations easier for everyone! 🚀

This repository demonstrates how to use an ESP32 microcontroller as a Bluetooth receiver to interface with a **Bluetooth Page Turner Remote Control**. The ESP32 can decode commands sent by the remote, enabling control of external systems such as robots or IoT devices.

## Features
- **Bluetooth Pairing:** Seamlessly connect an ESP32 to the Page Turner remote.
- **Command Decoding:** Detect and process button presses from the remote.
- **Robotic Integration:** Translate remote commands into robotic actions like movement or activation.

## Use Case
This setup is ideal for projects that require a simple, wireless control mechanism for robots, tools, or IoT systems. For example:
- Remotely control robot movements or tasks.
- Trigger custom actions in an IoT environment using a Bluetooth remote.

## Hardware Requirements
- [ESP32 Development Board](https://www.espressif.com/en/products/socs/esp32) (Bluetooth receiver).
- [Page Turner Bluetooth Remote Control](https://www.amazon.fr/dp/B0C6KZTHLC?ref=ppx_yo2ov_dt_b_fed_asin_title&th=1&language=en_GB) (Bluetooth emitter).

https://www.amazon.fr/dp/B0C6KZTHLC?ref=ppx_yo2ov_dt_b_fed_asin_title&th=1&language=en_GB


| ESP32 Development Board                                                                 | Page Turner Bluetooth Remote Control                                                                 |
| --------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------- |
| ![ESP32 Board](https://asset.conrad.com/media10/isa/160267/c1/-/fr/002490159PI00/image.jpg?x=1000&y=1000&format=jpg&ex=1000&ey=1000&align=center) | ![Bluetooth Remote](https://m.media-amazon.com/images/I/51KBmWkEMBL._AC_SL1500_.jpg) |


