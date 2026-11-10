# Radial Heat Conduction Verification

## Test Description

&nbsp;

This verification case checks the radial heat conduction between subchannels in the triangular-lattice solver, [TriSubChannel1PhaseProblem.md]. In the enthalpy equation of [subchannel_theory.md] each gap $ij$ between subchannels $i$ and $j$ conducts

\begin{equation}
Q_{ij,k} = k \frac{S_{ij,k} \eta_{ij}}{L_{ij}} (T_{i,k} - T_{j,k}),
\end{equation}

where $k$ is the fluid thermal conductivity and $S_{ij,k} = w_{ij} \Delta z_k$ is the gap area. The gap width $w_{ij}$ is $p - D$ between two pins and the duct-to-pin gap where the gap faces the duct, with pitch $p$ and pin diameter $D$. The centroid distance $L_{ij}$ is $p / \sqrt{3}$ for gaps that involve an interior subchannel and $p$ otherwise, and the shape factor is

\begin{equation}
\eta_{ij} = 0.66 \frac{p}{D} \left( \frac{w_{ij}}{D} \right)^{-0.3}.
\end{equation}

The problem isolates the conduction term. It consists of a 7-pin assembly with $p = 12$ mm, $D = 10$ mm, a duct flat-to-flat distance of 35 mm, and a heated length of 0.5 m divided into 20 axial cells. The pins are unheated and bare. Turbulent mixing is disabled ($\beta = 0$, $C_T = 0$), and a lateral form loss coefficient $K_{ij} = 10^8$ suppresses diversion crossflow. The fluid properties are constant and close to those of sodium at 660 K: $\rho = 850$ kg/m$^3$, $c_p = 1270$ J/kg-K, $k = 70$ W/m-K, and $\mu = 2.5 \times 10^{-4}$ Pa-s, so $h = c_p T$. The duct heats the subchannels with a heat flux of $2 \times 10^4$ W/m$^2$ where $x > 0$ and cools them with the same flux where $x < 0$, so the assembly receives no net heat and heat crosses it only by conduction through the gaps. The inlet mass flux is 5 kg/m$^2$-s, low enough that the outlet temperatures span about 50 K.

The reference solution solves the same conductance network independently of SCM. It rebuilds the 24 gaps of the assembly from the subchannel positions, assembles the conductances above, adds the duct heat of each edge and corner subchannel as the heat flux times its peripheral duct width, and marches the enthalpy cell by cell, implicitly in the conduction term. It takes only the subchannel positions, flow areas, and face mass flow rates from SCM, and not the SCM matrices. The flow split develops over the first cells, where the axial mass flow rate redistributes between subchannels; the reference solution uses the SCM face mass flow rates but does not model the enthalpy carried by that redistribution.

## Results

&nbsp;

[scm-radial-conduction] compares the outlet subchannel temperatures. Heat flows from the heated corner subchannel at $x = 18$ mm, 24.8 K above the inlet temperature, to the cooled corner subchannel at $x = -18$ mm, 24.8 K below it, and the subchannels on $x = 0$ stay at the inlet temperature. SCM and the network solution agree within $3.4 \times 10^{-3}$ K, about $10^{-4}$ of the temperature spread. The remaining difference is largest in the interior subchannels and comes from the flow redistribution over the development length.

!media media_scripts/verification/radial_conduction_verification/scm_radial_conduction.py
    image_name=scm_radial_conduction_7pin.png
    style=width:100%;margin-bottom:2%;margin:auto;
    id=scm-radial-conduction
    caption=Left: outlet subchannel temperature above the inlet temperature from SCM and from the independent network solution, versus the subchannel centroid position $x$. Right: difference between SCM and the network solution.

## Input file

&nbsp;

The verification problem uses the following input file:

!listing /verification/radial_conduction_verification/radial_conduction_7pin.i language=moose

The test `test_radial_conduction.py` runs it, solves the conductance network, and checks that the outlet temperatures agree within $4 \times 10^{-3}$ K:

!listing /verification/radial_conduction_verification/test_radial_conduction.py language=python
