import os

# Get all file names from diretory
file_names = os.listdir('channel_meshes')
block_names = []
for file in file_names:
    block_names.append(file.replace(".csv",""))

block_names = sorted(block_names)
print("Number of channels: " + str(len(block_names)) + " \n")

blocks_fw = [block for block in block_names if "OB_FW" in block]
print("Number of FW channels: " + str(len(blocks_fw)) + " \n")

blocks_shell = [block for block in block_names if "OB_shell" in block]
print("Number of shell channels: " + str(len(blocks_shell)) + " \n")

blocks_plate = [block for block in block_names if "plate" in block]
print("Number of plate channels: " + str(len(blocks_plate)) + " \n")

with open('positions_fw.txt', 'w') as f:
   # FW channels
   for block in blocks_fw:
       f.write("0 0 0\n")

with open('positions_shell.txt', 'w') as f:
   # Shell channels
   for block in blocks_shell:
       f.write("0 0 0\n")

with open('positions_plate.txt', 'w') as f:
   # Plate channels
   for block in blocks_plate:
       f.write("0 0 0\n")
