
# Enthalpy mixing model Verification

## Test Description

&nbsp;

This verification problem is the same used in [!cite](CTF-Verification). This case presents a problem where the effects of turbulent mixing are clearly discernible and quantifible. Turbulence causes both momentum and enthalpy mixing through the terms:

\begin{equation}
Drag_{ij} = -C_{T}\sum_{j} w'_{ij}\Delta U_{ij }
\end{equation}

\begin{equation}
h'_{ij} = \sum_{j} w'_{ij}\Delta h_{ij}
\end{equation}

Because the model for turbulent mixing is gradient-driven based in $\Delta h, \Delta U$, in order to observe the effects of turbulence, it is necessary to make a gradient in either energy or momentum. It is easier to focus on the energy equation and deactivate the density calculation. The problem geometry consists of two identical channels connected by a gap and is seen in [enthalpy].

!media subchannel/v&v/enthalpy.png
    style=width:30%;margin-bottom:2%;margin:auto;
    id=enthalpy
    caption=Enthalpy mixing model verification problem geometry

To test the turbulent mixing model, the temperature of one channel is raised by 10 degrees Celsius. Turbulent enthalpy mixing will transfer heat from the hot channel to the cold channel. The solution given by the code is then compared to the analytical solution:

\begin{equation}
h_1 = \frac{(h_{1,in} + h_{2,in})}{2} -  \frac{1}{2}(h_{2,in} - h_{1,in})\exp(-\frac{2 \frac{dw_{12}'}{dz}}{\dot{m}} z)
\end{equation}

\begin{equation}
h_2 = \frac{(h_{1,in} + h_{2,in})}{2} + \frac{1}{2}(h_{2,in} - h_{1,in})\exp(-\frac{2 \frac{dw_{12}'}{dz}}{\dot{m}} z)
\end{equation}

## Results

The analytical solution is compared with the code results in [enthalpy-ver]. The code results are in good agreement with the analytical solution.

!media subchannel/v&v/enthalpy-ver.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=enthalpy-ver
    caption=Enthalpy distribution in the axial direction

## Input file

To run the enthalpy mixing model verification problem use the following input file:

!listing /examples/verification/enthalpy_mixing_verification/two_channel.i language=cpp

The solution will be projected to the 3D mesh created by the following input file:

!listing /examples/verification/enthalpy_mixing_verification/3d.i language=cpp