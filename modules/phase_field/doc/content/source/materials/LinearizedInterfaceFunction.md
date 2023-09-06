# LinearizedInterfaceFunction

!syntax description /Materials/LinearizedInterfaceFunction

## Overview

!media /phase_field/linearized_interface_function.png style=width:49%;margin-left:1%;float:right;
       caption=Figure 1: Example of order parmeter values ($\phi_1$, $\phi_2$) and the linear preconditioning transformed variables ($\psi_1$, $\psi_2$),

This material defines the linearized interface substitution defined in [!cite](glasner2001nonlinear). This substitution converts the profile from a nonlinear shape to a linear shape, as shown in Fig. 1, requiring fewer elements across the interface to accurately resolve.

The transformation implemented in this material is defined as

\begin{equation}
    \phi_i = \frac{1}{2} \left[ 1 + \tanh\left( \frac{\psi_i}{\sqrt{2}} \right) \right],
\end{equation}

where $\phi_i$ is the order parameter and $\psi_i$ is the transformed variable. The function is implemented using the [ExpressionBuilder](/ExpressionBuilder.md) capability in MOOSE, such that all the derivatives of the function are calculated analytically.

## Example Input File Syntax

The material is used via the syntax shown below:

!listing modules/phase_field/test/tests/grain_growth_w_linearized_interface/grain_growth_linearized_interface.i block=Materials/gr0

This material is also generated as part of the automated syntax implemented in [GrainGrowthLinearizedInterfaceAction](/GrainGrowthLinearizedInterfaceAction.md).

!! Describe and include an example of how to use the LinearizedInterfaceFunction object.

!syntax parameters /Materials/LinearizedInterfaceFunction

!syntax inputs /Materials/LinearizedInterfaceFunction

!syntax children /Materials/LinearizedInterfaceFunction
