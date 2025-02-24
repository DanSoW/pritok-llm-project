import pyautogui
import time
import math

i = 0

while i < 50:
    pyautogui.moveTo(500 + 4 * i, 500 + 50 * math.cos(i))
    time.sleep(20 / 1000)
    i += 1