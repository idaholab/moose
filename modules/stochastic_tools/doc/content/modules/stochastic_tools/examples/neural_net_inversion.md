# Using a neural network IC

This example goes over how to train a neural network to invert concentration from chemical potential, for creating a sub-concentration IC for the KKS model. We use the [NeuralNetworkUserObject.md] and [NeuralNetworkIC.md].

## Overview

Evaluating a neural network requires two objects: NeuralNetworkUserObject and NeuralNetworkIC. The user object parses an XML structure file to reconstruct the trained neural network. It can then be evaluated by passing input parameter values, using the NeuralNetworkIC. The output values can then be assigned to the desired Variable or AuxVariable as an IC.

## Training a neural network on MOOSE Exodus data

We will first go over how to run a [modules/phase_field/MultiPhase/KKS.md]  model simulation and train a PyTorch neural network on the output data. KKS simulations numerically solve the equation

\begin{equation}
  \mu_i = \frac{dF^\beta(c^\beta_0,c^\beta_1,..)}{dc^\beta_i}
\end{equation}

To compute the value of $c^\beta_i$ given $F^\beta(c^\beta_0,c^\beta_1,..)$ is the free energy of phase $\beta$ as a function of the sub-concentrations of components. Usually, we solve KKS simulations using the Newton method solver. To get good convergence with the Newton method, a good initial condition is required. Manually providing these initial conditions is tedious for complex free energy functions with a large number of components. Since neural networks excel at providing approximate solutions to problems, they are a good candidate for automating the initial condition specification. As an example case, we look at an ideal solution model KKS simulation. This simulation is a toy model for the corrosion of an Ni-Cr alloy in a molten salt.

!listing examples/surrogates/kks_training.i

This simulation also requires the phase-field module in MOOSE to run. We will use the $MOOSE_DIR/modules/combined/combined-opt file to run this input file. The resulting output Exodus file should get stored in the examples/surrogates/kks_training folder.

Next, we run the neural_network_training.py script to train the neural network on the output file.

!listing examples/surrogates/neural_network_training.py

To run this training script, PyTorch needs to be installed with your Conda installation. If you do not already have PyTorch installed, run the following command in your MOOSE directory:

```language=bash
conda install -c pytorch pytorch
```

The script provided in this example uses the exodusReader.py and NeuralNet_to_XML.py scripts located in $MOOSE_DIR/python/ExodusNNTrainer directory. The exodusReader script provides cell values of the requested variables from the exodus file. The NeuralNet_to_XML script converts the PyTorch neural network model into an XML script that can be parsed within MOOSE using pugixml.

To execute the training python script, we only need to run the command:

```language=bash
python neural_network_training.py
```

If the exodusReader or NeuralNet_to_XML modules fail to load, you need to add MOOSE_DIR/python to your PATH. If everthing works, you will see the epochs and loss function printed out as the neural net is trained. We have set our net to stop when the loss function reaches 1e-2. This accuracy is sufficient for the purposes of creating an IC for this KKS model. This script runs on a GPU, if available on your system, and a CPU otherwise. For more information on writing PyTorch scripts, look up the official documentation and tutorials.

Once the script is finished running, you should have two more files in the kks_training director: NN_KKS_IC.pt and NN_KKS_IC.XML . We will use the XML file to parse the neural network into MOOSE. To parse the neural network within MOOSE, we need to add a [NeuralNetworkUserObject.md].

!listing examples/surrogates/neural_network_IC.i block=UserObjects

We replace the ICs for the sub-concentrations in the kks_training.i input file with [NeuralNetworkIC.md].

!listing examples/surrogates/neural_network_IC.i block=ICs

The inputs to the NeuralNetworkIC are the input variables, index of the output array to feed to the variable that we are providing the IC for, and the variable name itself. We run this simulation the exact same way as the first simulation, and compare the wall times against the simulation time for both models.

!media media/KKS_vs_NN_IC_KKS.png style=width:50%; caption=Performance comparison of KKS simulation with and without NeuralNetworkIC

Using the [NeuralNetworkIC.md] condition reduces the simulation time. This is achieved by improving the convergence rate for the first time step, since we are providing a good initial guess to the Newton solver.
