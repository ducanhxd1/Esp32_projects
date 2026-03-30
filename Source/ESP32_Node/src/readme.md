# Hệ thống ESP32 ESP-NOW + Smoke + SHT31 + Deep Sleep

## Xây dựng hệ thống gồm nhiều node ESP32

- Tự động kết nối bằng ESP-NOW
- Đọc:
    - Cảm biến khói (BM22S2021) qua Uart
    - Cảm biến SHT31 qua I2C
- Tiết kiệm năng lượng bằng __Deep Sleep__
- Thức dậy khi:
    - Timer (~2 giờ)
    - Hoặc chân cảm biến khói HIGH
- Gửi data về Master ESP32

## So do khoi he thong
```
[Node ESP32]  ---> ESP-NOW --->  [Master ESP32]
   |                                  |
 Smoke + SHT31                 Nhận + xử lý + log
 DeepSleep

```
