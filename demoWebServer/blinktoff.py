from blinkt import set_pixel, set_brightness, show, clear, set_clear_on_exit
from time import sleep
from math import log
 
bright = 1.0
 
for j in range(8):
    set_pixel(j,255,255,255)
   
 
for i in range (1,30):
    l = log(1+1.718*i/30.0)
    set_brightness(bright-(bright*l))
    show()
    sleep(0.001)
 
set_clear_on_exit(False)

