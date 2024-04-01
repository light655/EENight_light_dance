from midi_array import *
import cv2
from PIL import Image


video_name = "simulation.mp4"
video = cv2.VideoWriter(video_name, 0, 24, (640, 480))
max_time = max(max(A_t), max(B_t), max(C_t))
max_frame = max_time // 41667 + 1
index_A = 0
index_B = 0
index_C = 0

for i in range(max_frame):
    t = i * 41667
    # do something