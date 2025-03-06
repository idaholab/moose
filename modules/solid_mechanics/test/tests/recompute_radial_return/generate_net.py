import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import DataLoader, TensorDataset

print(torch.__version__)
torch.manual_seed(42)

# This is an example of a neural network that will determine the dislocation density
# based on strain and temperature (might not be physical, this is just an example)
class MyDDNet(nn.Module):
    def __init__(self):
        super(MyDDNet, self).__init__()

        # We save the normalization parameters into the buffer so we can 
        # load them in C++
        self.register_buffer('mean', torch.tensor([2.0000e-03,
        3.1500e+02]))
        self.register_buffer('std', torch.tensor([1.8257e-03,
        1.2910e+01]))

        # We have two linear layers, we can add more if needed
        self.layer1 = nn.Linear(2, 4)
        self.output_layer = nn.Linear(4, 1)

    def forward(self, x):
        x = (x - self.mean) / self.std
        x = self.layer1(x)
        x = self.output_layer(x)
        return x

# Alright, time to train a neural net
model = MyDDNet()

# These are the inputs for the learning process, not a lot so far. 
# Feel free to extend.
strain = [0.0,  0.001, 0.003, 0.004]
temperature =[300.0, 310.0, 320.0, 330.0]
dd = [[0.1, 0.2, 0.3, 0.4]]

# Converting things to tensors
input_tensor = torch.tensor([strain,temperature]).T
output_tensor = torch.tensor(dd).T

# Setting up loss and optimizer
criterion = nn.MSELoss()
optimizer = optim.Adam(model.parameters(), lr=0.1)

# Data loader just in case, we might not need it at this stage
dataset = TensorDataset(input_tensor, output_tensor)
dataloader = DataLoader(dataset, batch_size=4, shuffle=True)

# Main training loop
num_epochs = 100
for epoch in range(num_epochs):
    for inputs, targets in dataloader:
        # Forward pass
        outputs = model(inputs)
        loss = criterion(outputs, targets)
        
        # Backward pass and optimization
        optimizer.zero_grad()
        loss.backward()
        optimizer.step()
    
        print(f'Epoch [{epoch+1}/{num_epochs}], Loss: {loss.item():.4f}')

print("Training complete.")

print("Saving NN.")
scripted_model = torch.jit.script(model.double())
scripted_model.save("my_dd_net.pt")
