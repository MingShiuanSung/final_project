import pyb
import sensor, image, time, math
import os, tf

uart = pyb.UART(3,9600,timeout_char=1000)
uart.init(9600,bits=8,parity = None, stop=1, timeout_char=1000)
tmp = ""

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time = 2000)
sensor.set_windowing((240, 240))       # Set 240x240 window. //add
sensor.set_auto_gain(False)  # must turn this off to prevent image washout...
sensor.set_auto_whitebal(False)  # must turn this off to prevent image washout...
clock = time.clock()

def image_classification():
   sensor.reset()                         # Reset and initialize the sensor.
   sensor.set_pixformat(sensor.RGB565)    # Set pixel format to RGB565 (or GRAYSCALE)
   sensor.set_framesize(sensor.QVGA)      # Set frame size to QVGA (?x?)
   sensor.set_windowing((240, 240))       # Set 240x240 window.
   sensor.skip_frames(time=2000)          # Let the camera adjust.

   labels = ['3', '4', '0', 'other']

   img = sensor.snapshot()
   img.save("MNIST.jpg")

   for obj in tf.classify('/model_demo.tflite',img, min_scale=1.0, scale_mul=0.5, x_overlap=0.0, y_overlap=0.0):
      img.draw_rectangle(obj.rect())
      img.draw_string(obj.x()+3, obj.y()-1, labels[obj.output().index(max(obj.output()))], mono_space = False)
   return labels[obj.output().index(max(obj.output()))]

while(True):
   clock.tick()
   img = sensor.snapshot()

   # Data matrix
   img.lens_corr(1.8) # strength of 1.8 is good for the 2.8mm lens.

   matrices = img.find_datamatrices()
   for matrix in matrices:
      img.draw_rectangle(matrix.rect(), color = (255, 0, 0))
      print_args = (matrix.rows(), matrix.columns(), matrix.payload(), (180 * matrix.rotation()) / math.pi, clock.fps())
      uart.write(("Matrix [%d:%d], Payload \"%s\", rotation %f (degrees), FPS %f\r\n" % print_args).encode())
   if not matrices:
      uart.write(("FPS %f\r\n" % clock.fps()).encode())

   # MNIST classification
   a = uart.readline()
   if a is not None:
      tmp += a.decode()

   if tmp == "image_classification":
      tmp =""
      label = image_classification()
      uart.write(label.encode())
