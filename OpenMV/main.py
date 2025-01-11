import sensor
import time
import ml
from ml.utils import NMS
import math
import image
from pyb import UART
import ustruct
from machine import LED


led = LED("LED_BLUE")

led.on()
time.sleep_ms(500)
sensor.reset()  # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565)  # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)  # Set frame size to QVGA (320x240)
sensor.skip_frames(time=200)  # Let the camera adjust.
sensor.set_auto_gain(False)
sensor.set_auto_whitebal(False)
sensor.set_auto_exposure(True)
time.sleep(2)
sensor.set_auto_exposure(False,exposure_us=5000)
led.off()

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

thre_red = (50, 80, 20, 50, -10, 30)
thre_green = (0, 90, -40, -10, -20, 20)
print("Start")
clock = time.clock()
IsExposure = False
while True:
    while uart.any() == 0:
        pass
    OpType=int(uart.readchar())
    # print(OpType)
    if OpType == 0 or OpType == 1:
        if IsExposure:
            sensor.set_auto_exposure(False,exposure_us=5000)
            IsExposure = False
            sensor.skip_frames(time=100)
    if OpType == 0:
        medians = []
        victim_cnt = 0
        for i in range(10):
            img = sensor.snapshot()
            pred = model.predict([img], callback=fomo_post_process)
            Temp = []
            # print(pred)
            if(len(pred)<3): continue

            for (x, y, w, h), score in pred[2]:
                center_x = math.floor(x + (w / 2))
                center_y = math.floor(y + (h / 2))
                Temp.append((center_x,center_y))
            victim_cnt += len(Temp)
            Temp.sort()
            medians.append(Temp[len(Temp)//2][0])
        if victim_cnt > 8:
            medians.sort()
            uart.write(ustruct.pack('B',1))

            uart.write(ustruct.pack('B',medians[len(medians)//2]//10))
        else:
            uart.write(ustruct.pack('B',0))
    elif OpType == 1:
        medians = []
        victim_cnt = 0
        for i in range(10):
            img = sensor.snapshot()
            pred = model.predict([img], callback=fomo_post_process)
            Temp = []
            if (len(pred)<2): continue
            for (x, y, w, h), score in pred[1]:
                center_x = math.floor(x + (w / 2))
                center_y = math.floor(y + (h / 2))
                Temp.append((center_x,center_y))
            victim_cnt += len(Temp)
            Temp.sort()
            medians.append(Temp[len(Temp)//2][0])
        if victim_cnt > 8:
            medians.sort()
            uart.write(ustruct.pack('B',1))
            uart.write(ustruct.pack('B',medians[len(medians)//2]//10))
        else:
            uart.write(ustruct.pack('B',0))
    elif OpType==2 or OpType==3:
        if not IsExposure:
            sensor.set_auto_exposure(False,exposure_us=50000)
            IsExposure = True
            sensor.skip_frames(time=100)
        img = sensor.snapshot()
        obj = img.find_blobs([thre_red if OpType == 3 else thre_green],merge=True,area_threshold=500,pixels_threshold=500)
        max_pixel = 0
        max_i = -1
        for i,b in enumerate(obj):

            if max_pixel < b[4]:
                max_pixel = b[4]
                max_i = i
        if max_pixel > 700:
            uart.write(ustruct.pack('B',1))
            uart.write(ustruct.pack('B',obj[max_i][5]//10))
            uart.write(ustruct.pack('B',obj[max_i][2]//10))

        else:
            uart.write(ustruct.pack('B',0))
    uart.flush()
    led.off()
