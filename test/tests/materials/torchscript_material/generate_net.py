import torch
import torch.nn as nn

torch.manual_seed(42)

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

scripted_model = torch.jit.script(MyNet().double())
scripted_model.save("my_net.pt")
