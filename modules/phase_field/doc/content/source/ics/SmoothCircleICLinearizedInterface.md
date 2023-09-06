# SmoothCircleICLinearizedInterface

!syntax description /ICs/SmoothCircleICLinearizedInterface

## Overview

!media media/phase_field/SmoothCircleLinearizedInterface_psi.png
  style=width:40%;margin-left:1%;float:right;
  caption=Figure 2: Initial condition for linearized interface variable from SmoothCircleICLinearizedInterface with bounds of $\pm5$

!media media/phase_field/SmoothCircleLinearizedInterface_phi.png
  style=width:40%;margin-left:1%;float:right;
  caption=Figure 1: Initial condition for order parameter from SmoothCircleIC


This IC is for use with the linearized interface grain growth model. It converts the output from [SmoothCircleIC](/SmoothCircleIC.md) from an order parameter $\phi_i$ for the traditional grain growth model (see Fig. 1) to the changed variable for linearized interface variable $\psi_i$ (see Fig. 2). It uses the function from [!cite](glasner2001nonlinear):

\begin{equation}
  \phi_i = \frac{1}{2} \left[ 1 + \tanh\left( \frac{\psi_i}{\sqrt{2}} \right) \right],
\end{equation}

So, the equation for the initial condition of the transformed variables $\psi_i$ is

\begin{equation}
  \psi_i = \sqrt{2} \tanh^{-1} (2 \phi_i - 1).
\end{equation}

The initial condition also has to consider the bounds values, $\pm b$. When the function value of the order parameter initial condition is outside the bounds, the transformed variable value is equal to the bounds value. However, to make this evaluation, the bounds values have to be converted to order parameter values, such that:

- For $\phi_i > \frac{1}{2} \left[ 1 + \tanh\left( \frac{b}{\sqrt{2}} \right) \right]$, $\psi_i = b$.
- For $\phi_i <\ \frac{1}{2} \left[ 1 + \tanh\left( \frac{-b}{\sqrt{2}} \right) \right]$, $\psi_i = -b$.

## Example Input File Syntax

The input file syntax for the IC is:

!listing modules/phase_field/test/tests/grain_growth_w_linearized_interface/grain_growth_linearized_interface.i block=ICs/phi0_IC

!syntax parameters /ICs/SmoothCircleICLinearizedInterface

!syntax inputs /ICs/SmoothCircleICLinearizedInterface

!syntax children /ICs/SmoothCircleICLinearizedInterface
