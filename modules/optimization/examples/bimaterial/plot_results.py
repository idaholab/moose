import os
import sys
import matplotlib.pyplot as plt

plt.figure()

model_calls = []
phi = []
with open("gold/pest.rec", "r") as f:
    for line in f:
        line = line.strip()
        if line.startswith("Model calls so far"):
            model_calls.append(int(line.split(":")[-1]))
        if line.startswith("Starting phi for this iteration"):
            phi.append(float(line.split(":")[-1]))
plt.semilogy(model_calls, phi, 'o-', label = "PEST")

forwards = [0]
obj_fcn = []
with open("gold/main_out_optInfo_0001_lmvm.csv", "r") as f:
    data = f.readlines()[1:]
for line in data:
    cnorm, current_iterate, function_value, gnorm, gradient_iterate, hessian_iterate, objective_iterate, xdiff = line.strip().split(",")
    forwards.append(forwards[-1] + int(objective_iterate))
    obj_fcn.append(float(function_value))
forwards.pop(0)
plt.semilogy(forwards, obj_fcn, 'o-', label = "TAO: LMVM")

forwards = [0]
obj_fcn = []
with open("gold/main_out_optInfo_0001_nm.csv", "r") as f:
    data = f.readlines()[1:]
for line in data:
    cnorm, current_iterate, function_value, gnorm, gradient_iterate, hessian_iterate, objective_iterate, xdiff = line.strip().split(",")
    forwards.append(forwards[-1] + int(objective_iterate))
    obj_fcn.append(float(function_value))
forwards.pop(0)
plt.semilogy(forwards, obj_fcn, 'o-', label = "TAO: NM")


plt.legend()
plt.grid()
plt.xlim([0, plt.xlim()[1]])
plt.xlabel("Number of model solves")
plt.ylabel("Objective function")
plt.title("Comparison of inverse methods for the bimaterial example")
plt.savefig("comparison.png", bbox_inches = 'tight')
plt.show()

    
