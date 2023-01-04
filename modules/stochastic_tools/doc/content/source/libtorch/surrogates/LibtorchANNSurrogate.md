# LibtorchANNSurrogate

!if function=hasLibtorch()
!syntax description /Surrogates/LibtorchANNSurrogate

## Overview

The details of a simple feedforward neural network is discussed in [utils/LibtorchArtificialNeuralNet.md].
This class is dedicated to evaluating the following function:

!equation id=nn-explicit
\textbf{y} = \sigma(\textbf{W}^{(n)}\sigma(\textbf{W}^{(n-1)}
\sigma( ... \sigma(\textbf{W}^{(1)}\textbf{x}+\textbf{b}^{(1)}) ... )+\textbf{b}^{(n-1)})+\textbf{b}^{(n)}),

which describes a neural network of $n$ layers. In this context, $\sigma$ denotes
an activation function, while $\textbf{x}$ and $\textbf{y}$ are the input and
output arguments. respectively. The weight matrices ($\textbf{W}$) and bias vectors
($\textbf{b}$) are optimized by [LibtorchANNTrainer.md] and are fixed in the evaluation phase.

## Example Input File Syntax

Let us consider an example where we evaluate the neural network trained
[here](LibtorchANNTrainer.md). For this, prepare another set of samples of
from the same parameter space:

!listing libtorch_nn/evaluate.i block=Samplers

Following this, we load the surrogate from a file saved by the trainer:

!listing libtorch_nn/evaluate.i block=Surrogates

And evaluate it using a reporter which uses the samples and the surrogate
to compute the approximate values of the target function at the new sample points:

!listing libtorch_nn/evaluate.i block=Reporters

!if! function=hasLibtorch()

!syntax parameters /Surrogates/LibtorchANNSurrogate

!syntax inputs /Surrogates/LibtorchANNSurrogate

!syntax children /Surrogates/LibtorchANNSurrogate

!if-end!

!else
!include libtorch/libtorch_warning.md
