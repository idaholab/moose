# PolycrystalColoringICLinearizedInterface

!syntax description /ICs/PolycrystalColoringICLinearizedInterface

## Overview

!media media/phase_field/PolycrystalColoringICLinearizedInterface_psi.png
  style=width:40%;margin-left:1%;float:right;
  caption=Figure 2: Initial condition for linearized interface variable from PolycrystalColoringICLinearizedInterface with bounds of $\pm5$

!media media/phase_field/PolycrystalColoringICLinearizedInterface_phi.png
  style=width:40%;margin-left:1%;float:right;
  caption=Figure 1: Initial condition for order parameter from PolycrystalColoringIC

This IC is for use with the linearized interface grain growth model. It converts the output from [PolycrystalColoringIC](/PolycrystalColoringIC.md) from an order parameter $\phi_i$ for the traditional grain growth model (see Fig. 1) to the changed variable for linearized interface $\psi_i$ (see Fig. 2). It works with the various [Polycrystal ICs](/PolycrystalICs.md).

For the linearized interface, it uses the function from [!cite](glasner2001nonlinear):

\begin{equation}
  \phi_i = \frac{1}{2} \left[ 1 + \tanh\left( \frac{\psi_i}{\sqrt{2}} \right) \right],
\end{equation}

So, the equation for the initial condition of the transformed variables $\psi_i$ is

\begin{equation}
  \psi_i = \sqrt{2} \tanh^{-1} (2 \phi_i - 1).
\end{equation}

The initial condition also has to consider the bounds values, $\pm b$ (defined by `bound_value`). When the function value of the order parameter initial condition is outside the bounds, the transformed variable value is equal to the bounds value. However, to make this evaluation, the bounds values have to be converted to order parameter values, such that:

- For $\phi_i > \frac{1}{2} \left[ 1 + \tanh\left( \frac{b}{\sqrt{2}} \right) \right]$, $\psi_i = b$.
- For $\phi_i <\ \frac{1}{2} \left[ 1 + \tanh\left( \frac{-b}{\sqrt{2}} \right) \right]$, $\psi_i = -b$.

## Example Input File Syntax

We never recommend using this IC directly, but rather creating the full set of ICs for all of the transformed variables using [PolycrystalColoringICAction](/PolycrystalColoringICAction.md).

!syntax parameters /ICs/PolycrystalColoringICLinearizedInterface

!syntax inputs /ICs/PolycrystalColoringICLinearizedInterface

!syntax children /ICs/PolycrystalColoringICLinearizedInterface
