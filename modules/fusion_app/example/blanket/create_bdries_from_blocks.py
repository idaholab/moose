import os

# Get all file names from diretory
file_names = os.listdir('channel_meshes')
block_names = []
for file in file_names:
    block_names.append(file.replace(".csv",""))

#print(block_names)
#print(len(block_names))
with open('create_bdries_from_blocks.i', 'w') as f:
   # FW channels
   f.write("[Mesh]\n")
   input = "fmg"
   # construct a deletion input block for each block 
   for block in block_names:
       f.write("  ["+ block +"]\n")
       f.write("    type = SideSetsBetweenSubdomainsGenerator \n")
       f.write("    input = " +input+ " \n")
       input = block
       if "shell" in block:
           f.write("    primary_block = " +"'"+"OB_shell"+"'"+ " \n")
       elif "plate" in block:
           f.write("    primary_block = " +"'"+"OB_radial_plate"+"'"+ " \n")
       else:
           f.write("    primary_block = " +"'"+"OB_FW_SW"+"'"+ " \n")
       f.write("    paired_block = " + "'" + block + "'" + " \n")
       f.write("    new_boundary = " + "'" + block + "'" + " \n")
       f.write("  []\n")

   f.write("  [block_deletion] \n")
   f.write("    type = BlockDeletionGenerator \n")
   f.write("    input = " + input + "\n")
   f.write("    block = " + "'" + " ".join(block_names) + "'" + "\n")
   f.write("  []\n")


   f.write("[]\n")
