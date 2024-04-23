# Introduction to SubChannel

!---

## Motivation

!row!
!col! width=50%

!media figures/CFD.jpg

!col-end!

!col! width=50%

- CFD DNS calculations such shown in the left are computationally prohibitive.

!col-end!
!row-end!

!row!
!col! width=50%

!media figures/CTF.jpg

!col-end!

!col! width=50%

- SubChannel calculations such shown in the left (CTF: Isometric view of core mixture mass flux distribution) are practical and fast.

- We want to integrate a subchannel code in MOOSE.

- This will allow multiphysics and multiscale coupling within MOOSE.

- Access numerical solvers supported by PETSc and MOOSE.

!col-end!
!row-end!

!---

## Subchannel Overview

!row!

!col! width=50%

- The system level thermal hydraulic analysis codes like RELAP, RETRAN, ATHLET are used to get the balance of plant behavior.

- The results of this analysis give the boundary conditions used for the core level/component analysis.

- The detailed analysis of the reactor core is performed using the subchannel thermal hydraulic codes.

!col-end!

!col! width=50%

!media figures/shapes.png

!col-end!

!row-end!

!row!
!col! width=50%

!media figures/scales.png

!col-end!

!col! width=50%

- Subchannel codes are thermal-hydraulic codes  that offer an efficient compromise for the simulation of a nuclear reactor core, between CFD and system codes

!col-end!
!row-end!

!---

## Subchannel Model

!row!

!col! width=50%

- Subchannel discretization principle:

  - The rod bundle cross section is divided into flow subchannels.

  - The length of the bundle is divided into finite intervals.

  - The result is a set of control volumes that represent the flow region of the rod bundle.

- Limitations:

  - Local distributions within a subchannel are not considered. This eliminates the need for zero slip boundary conditions at solid surfaces.

  - Needs correlations to model wall friction and heat transfer.

!col-end!

!col! width=40%

!media figures/ControlVolume1.png

!col-end!

!row-end!

!---

## Subchannel Model 2

!row!

!col! width=50%

- The governing equations are derived by integrating and averaging the conservation equations (mass, momentum, energy) over the cell volumes $V_i ,V_{ij}$

- The sub-channel thermal hydraulic analysis solves the conservation equations of mass, momentum and energy on the specified control volumes.

- The control volumes are connected in both axial and radial directions.

!col-end!

!col! width=50%

!media figures/ControlVolume2.png

!col-end!

!row-end!

!media figures/Vij.png
    style=width:25%;margin-bottom:2%;margin:auto;

!---

## Subchannel Model Governing Equations

### Mass conservation equation

\begin{equation}
\label{mass-conservation-equation}
\frac{d\rho_i}{dt} V_i +\Delta \dot{m_i}+\sum_{j} w_{ij} = 0
\end{equation}

### Axial momentum conservation equation

\begin{equation}
\label{conservation-axial-momentum}
\frac{d \dot{m}_i }{dt} \Delta Z+ \Delta(\frac{\dot{m_i}^2}{S_i \rho_i}) + \sum_{j}w_{ij} U^\star =
-S_i  \Delta P_i+
Friction_i + Drag_{ij} - g  \rho_i  S_i \Delta Z
\end{equation}

### Lateral momentum conservation equation

\begin{equation}
\label{lateral-momentum}
\frac{dw_{ij}}{dt} L_{ij} + \frac{L_{ij}}{\Delta Z} \Delta (w_{ij} \bar{U}) = - S_{ij}  \Delta P_{ij} + Friction_{ij}
\end{equation}

### Enthalpy conservation equation

\begin{equation}
\label{enthalpy-conservation}
\frac{d\left\langle \rho h\right\rangle_i }{dt}V_i + \Delta (\dot{m}_i h_i)  + \sum_{j} w_{ij} h^\star  + h'_{ij} = q'_i \Delta Z
\end{equation}

!---

## Subchannel Model Closure Models

!row!

!col! width=50%

### Axial direction friction term

\begin{equation}
\small
    Friction_i = -\frac{1}{2} K_i \frac{\dot{m}_{i} |\dot{m}_{i}|}{S_{i} \rho_i }
\end{equation}

!col-end!

!col! width=50%

### Lateral direction friction term

\begin{equation}
\small
    Friction_{ij}  =  -\frac{1}{2} g_{ij} \Delta Z K_{ij} \rho_{} |u_{ij}| u_{ij} = - \frac{1}{2}K_{ij} \frac{w_{ij}|w_{ij}|}{S_{ij} \rho^\star}
\end{equation}

!col-end!

!row-end!

!row!

!col! width=50%

### Friction factor

\begin{equation}
\small
    f_w \rightarrow
    \begin{cases}
    \frac{1}{64} , & Re < 1\\
    \frac{64}{Re}, &1 \leq Re<5000\\
    0.316 Re^{-0.25}, &5000 \leq Re < 30000\\
    0.184 Re^{-0.20}, &30000 \leq Re
    \end{cases}
\end{equation}

!col-end!

!col! width=50%

### Turbulent momentum diffusion

\begin{equation}
\small
    Drag_{ij} = -C_{T}\sum_{j} w'_{ij}\Delta U_{ij } = -C_{T}\sum_{j} w'_{ij}\big[ \frac{\dot{m_i}}{\rho_iS_i} - \frac{\dot{m_j}}{\rho_j S_j}\big]
\end{equation}

!col-end!

!row-end!

### Turbulent enthalpy diffussion

\begin{equation}
\small
    h'_{ij} = \sum_{j} w'_{ij}\Delta h_{ij} = \sum_{j} w'_{ij}\big[ h_i - h_j  \big]
\end{equation}

### Turbulent crossflow

\begin{equation}
w'_{ij} = \beta S_{ij} \bar{G}, ~\frac{dw'_{ij}}{dz} = \frac{w'_{ij}}{\Delta Z}=\beta g_{ij} \bar{G}.
\end{equation}

!---

## Subchannel Model Algorithm

!row!
!col! width=50%

!media figures/stencil2.png
    style=width:100%;

!col-end!

!col! width=50%

- The essense of the algorithm hinges on the construction of a combined residual function based on the lateral momentum equation.

- The main unknown variable in this non linear residual is the crossflow $w_{ij}$. The combined residual function calculates the non linear residual $f(w_{ij})$ after it updates the other main flow variables.

- Once the main flow variables converge in a block, the enthalpy conservation equation is solved. Using enthalpy, pressure and the equations of state, temperature $T_i$ and the fluid properties such as density $\rho_i$ and viscosity $\mu_i$ are calculated. After the fluid properties are updated, the solve is repeated until the temperature converges. Once the temperature solution converges the procedure is repeated for the next block downstream until all blocks are solved and pressure converges.

!col-end!
!row-end!

!---

## Subchannel Model Algorithm 2

There are three variations of the algorithm: explicit (default), implicit segregated and implicit monolithic.

### Explicit

This is the default algorithm, where the unknown flow variables are calculated in an explicit manner through their governing equations.

### Implicit segregated

In this case, the governing equations are recast in matrix form and the flow variables are calculated by solving the corresponding system. Otherwise, the solution algorithm is the same as in the default method.

### Implicit monolithic

In this case, the conservation equations are recast in matrix form and combined  into a single system. The solution algorithm is the same as in the default method, but the solver used in this version is a fixed point iteration instead of a Newton method. The system looks like this:

\begin{equation}
\begin{bmatrix}
\boldsymbol{M_{mm}} & 0 & \boldsymbol{M_{mw}} & 0\\
\boldsymbol{M_{pm}} & \boldsymbol{M_{pp}} & 0 & 0 \\
0 & \boldsymbol{M_{wp}} & \boldsymbol{M_{ww}} & 0 \\
0 & 0 & 0 & \boldsymbol{M_{hh}}
\end{bmatrix}
\times
\begin{bmatrix}
\vec{\dot{m}} \\
\vec{P} \\
\vec{w}\\
\vec{h}
\end{bmatrix} =
\begin{bmatrix}
\vec{b_m}\\
\vec{b_p} \\
\vec{b_w} \\
\vec{b_h}
\end{bmatrix}
\end{equation}
