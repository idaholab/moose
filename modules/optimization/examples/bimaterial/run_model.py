#!/usr/bin/python3
import subprocess

# because PEST operates on files, we have to get the diffusivities from the PEST file pest_params.csv
with open("pest_params.csv", "r") as f:
    ktop, kbot = f.readlines()[-1].strip().split(",")

# then feed them to model.i
subprocess.call(['../../isopod-opt', '-i', 'model.i', 'Outputs/csv=true', 'Materials/mat_top/prop_values=' + ktop, 'Materials/mat_bottom/prop_values=' + kbot])

# Now format the output as required by pest_instruction.ins
with open("model_out.csv", "r") as f:
    tme, T_bottom_left, T_bottom_mid, T_bottom_right, T_top, d_bot, d_top = f.readlines()[-1].strip().split(",")
with open("pest_to_read.txt", "w") as f:
    f.write("Data\n")
    f.write(" " + T_top + "                        \n")
    f.write(" " + T_bottom_mid + "                        \n")
    f.write(" " + T_bottom_left + "                       \n")
    f.write(" " + T_bottom_right + "                      \n")





