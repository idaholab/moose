import csv
import numpy as np

# This script is used to generate a CSV mesh file for a quarter

radius = 3.25
start_angle = 270
end_angle = 360
nnodes = 41

central_angle = end_angle - start_angle

arc_length = 2 * np.pi * radius * (central_angle / 360.)

interval = central_angle/(nnodes-1)


print(interval)

with open('quarter.csv', mode='w') as csv_file:
    fieldnames = ['x', 'y', 'z']
    writer = csv.DictWriter(csv_file, fieldnames=fieldnames)

    writer.writeheader()
    for i in range(nnodes):
      alpha = (i*interval + start_angle)*(np.pi/180)
      x = radius * np.cos(alpha)
      y = radius * np.sin(alpha)
      z = 0
      writer.writerow({'x': x, 'y': y, 'z': z})
