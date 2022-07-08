import os

# Get all file names from diretory
file_names = os.listdir('channel_meshes')
block_names = []
for file in file_names:
    block_names.append(file.replace(".csv",""))

shell_block = []
plate_block = []
FW_block = []

for block in block_names:
    if "shell" in block:
        shell_block.append(block)
    elif "plate" in block:
        plate_block.append(block)
    elif "FW" in block:
        FW_block.append(block)
    else:
        "wrong, wrong"

with open('create_bdry_blocks.i', 'w') as f:
   f.write("shell_channel = '" + " ".join(shell_block) + "' \n")
   f.write("plate_channel = '" + " ".join(plate_block) + "' \n")
   f.write("FW_channel = '" + " ".join(FW_block) + "' \n")
