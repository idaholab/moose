# %%
import json
import numpy as np
from matplotlib import pyplot as plt

file  = '/Users/dhulls/projects/moose/modules/stochastic_tools/test/tests/samplers/ParallelActiveLearning/BayesianOpt_out1_BayesOpt 2.json'
steps = 60
dim = 10
prop = 5
f = open(file,)
data = json.load(f)
outputs = np.zeros((steps, prop))
inputs = np.zeros((steps, prop, dim))
c1 = 0
for ii in np.arange(1,(steps+1),1):
    outputs[ii-1, :] = np.array(data["time_steps"][ii]['conditional']['outputs_required'])
    inputs[ii-1, :, :] = np.array(data["time_steps"][ii]['conditional']['inputs'])

plt.plot(np.maximum.accumulate(np.max(outputs[1:,:],axis=1)))
# plt.plot(np.minimum.accumulate(np.min(outputs[1:,:],axis=1)))
plt.xlabel('Iteration')
plt.ylabel('Objective value')

# %%
