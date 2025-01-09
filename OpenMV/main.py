

import sensorz
import time
import ml
from ml.utils import NMS
import math
import image
from pyb import UART
import ustruct


"""  todo 
輝度の自動調整をオフにする
メインマイコンから送られてきたデータに応じて、返すデータを変える処理を書く
緑、赤の閾値調整
ボールを x 座標でソート
ボールがあるべき y 座標の範囲を決めて、事前処理することでできるだけノイズを減らす
int をシリアルに送る際には byte 型に変換が必要なのでそれを書く ok
"""


sensor.reset()  # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565)  # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)  # Set frame size to QVGA (320x240)
sensor.skip_frames(time=2000)  # Let the camera adjust.
sensor.set_auto_gain(False)
sensor.set_auto_whitebal(False)

uart = UART(3,115200)
uart.init(115200)


min_confidence = 0.85
threshold_list = [(math.ceil(min_confidence * 255), 255)]

# Load built-in model
model = ml.Model("trained")
print(model)


colors = [  
    (255, 0, 0),
    (0, 255, 0),
    (255, 255, 0),
    (0, 0, 255),
    (255, 0, 255),
    (0, 255, 255),
    (255, 255, 255),
]


def fomo_post_process(model, inputs, outputs):
    n, oh, ow, oc = model.output_shape[0]
    nms = NMS(ow, oh, inputs[0].roi)
    for i in range(oc):
        img = image.Image(outputs[0][0, :, :, i] * 255)
        blobs = img.find_blobs(
        
            threshold_list, x_stride=1, area_threshold=1, pixels_threshold=1
        )
        for b in blobs:
            rect = b.rect()
            x, y, w, h = rect
            score = (
                img.get_statistics(thresholds=threshold_list, roi=rect).l_mean() / 255.0
            )
            nms.add_bounding_box(x, y, x + w, y + h, score, i)
    return nms.get_bounding_boxes()

thre_red = (0, 30, 0, 30, -3, 25)
thre_green = (15, 35, -14, -4, -4, 8)

clock = time.clock()
while True:
    while uart.any() == 0:
        pass
    OpType = int.from_bytes(UART.readchar(),'little')
    if OpType == 0:
        medians = []
        victim_cnt = 0
        for i in range(10):
            img = sensor.snapshot()
            pred = model.predict([img], callback=fomo_post_process)
            Temp = []
            for (x, y, w, h), score in pred[0]:
                center_x = math.floor(x + (w / 2))
                center_y = math.floor(y + (h / 2))
                Temp.append(tuple(center_x,center_y))
            victim_cnt += len(Temp)
            Temp.sort()
            medians.append(Temp[len(Temp)//2][0])
        if victim_cnt > 8:
            medians.sort()
            uart.write(ustruct.pack('B',1))
            uart.write(ustruct.pack('B',medians[4]//10))
        else:
            uart.write(ustruct.pack('B',0))
    elif OpType == 1:        
        medians = []
        victim_cnt = 0
        for i in range(10):
            img = sensor.snapshot()
            pred = model.predict([img], callback=fomo_post_process)
            Temp = []
            for (x, y, w, h), score in pred[0]:
                center_x = math.floor(x + (w / 2))
                center_y = math.floor(y + (h / 2))
                Temp.append(tuple(center_x,center_y))
            victim_cnt += len(Temp)
            Temp.sort()
            medians.append(Temp[len(Temp)//2][0])
        if victim_cnt > 8:
            medians.sort()
            uart.write(ustruct.pack('B',1))
            uart.write(ustruct.pack('B',medians[4]//10))
        else:
            uart.write(ustruct.pack('B',0))
    else:
        image = sensor.snapshot()
        max_i = -1
        max_pixel = 0
        for i,b in enumerate(img.find_blobs([thre_red if OpType == 2 else thre_green],pixels_threfold=1000,area_threshold=1000,merge=True)):
            if b[4] > max_pixel:
                max_i = i
                max_pixel = max_pixel
        uart.write(ustruct.pack('B',b[5]))
        uart.write(ustruct.pack('B',b[6]))