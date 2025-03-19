#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import torch
import torch.nn as nn

class MyNet(nn.Module):
    def __init__(self):
        super(MyNet, self).__init__()
        self.layer1 = nn.Linear(3, 5)
        self.relu1 = nn.ReLU()
        self.output_layer = nn.Linear(5, 1)

    def forward(self, x):
        x = self.layer1(x)
        x = self.relu1(x)
        x = self.output_layer(x)
        return x

model = MyNet().double()

# We need to initialize the parameters like this because the rng
# might result in slightly different results on different architectures
def init_weights(m):
    nn.init.constant_(m.weight, 0.1)
    if m.bias is not None:
        nn.init.constant_(m.bias, 0.15)

model = MyNet().double()
model.apply(init_weights)

scripted_model = torch.jit.script(model)
scripted_model.save("my_net.pt")
