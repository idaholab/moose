# NAFEMS Benchmark 1D Transient Heat Conduction T3

## Problem Description

A NAFEMS one-dimensional transient heat conduction problem
[!citep](nafems_std_bench) was created and run
using the heat conduction module in MOOSE. A description of the problem and a
summary of the results follows.

### Model

A bar of length 0.1 meters and cross section of 0.01 meters in width and depth
is modeled using uniform sized elements with either a coarse mesh (5 elements,
[fig:t3_coarse_mesh]) or a fine mesh (10 elements, [fig:t3_fine_mesh]). A
transient heat conduction simulation is performed with a total simulation time
of 32 seconds. The simulation was modeled using 1D, 2D and 3D element types.

!media media/heat_conduction/nafems_t3_coarse_mesh.png
       style=width:650px;margin-left:70px;float:center;
       id=fig:t3_coarse_mesh
       caption=Coarse mesh for NAFEMS T3 transient heat conduction model.

!media media/heat_conduction/nafems_t3_fine_mesh.png
       style=width:650px;margin-left:70px;float:center;
       id=fig:t3_fine_mesh
       caption=Fine mesh for NAFEMS T3 transient heat conduction model.

### Loading

Zero internal heat generation.

### Boundary Conditions

\begin{equation}
\begin{aligned}
\textsf{At time t} = 0,~&\textsf{All temperature} = 0^{\circ}\textsf{C}\\
\textsf{At time t} > 0,~&\textsf{At left end of domain, temperature} = 0^{\circ}\textsf{C}\\
&\textsf{At right end of domain, temperature} = 100 \sin(\pi t/40)^{\circ}\textsf{C}
\end{aligned}
\end{equation}

No heat flux perpendicular to domain

### Material Properties

\begin{equation}
\begin{aligned}
  \textsf{Conductivity} &= 35.0~\textsf{W/m}^{\circ}\textsf{C}\\
  \textsf{Specific Heat} &= 440.5~\textsf{J/kg}^{\circ}\textsf{C}\\
  \textsf{Density} &= 7200~\textsf{kg/m}^3
\end{aligned}
\end{equation}

### Reference Solution

The temperature at a position of 0.08 meters along the length of the bar at
the end of the simulation is the target solution. This value should be
$36.6^{\circ}\textsf{C}$.

## Results

The table below summarizes the temperature solutions for this problem using
the heat conduction module of MOOSE. The values in parentheses are the
difference between the MOOSE result and the target solution in percentages.

!table id=table:nafems_t3_results caption=Summary of Results
| Element Type | T, Coarse Mesh | T, Fine Mesh |
|--------------|--------------|--------------|
| EDGE2 | 40.3 (10.1%) | 36.9 ( 0.8%) |
| EDGE3 | 35.9 (-1.9%) | 36.1 (-1.4%) |
| QUAD4 | 40.3 (10.1%) | 36.9 ( 0.8%) |
| QUAD8 | 35.9 (-1.9%) | 36.1 (-1.4%) |
| QUAD9 | 35.9 (-1.9%) | 36.1 (-1.4%) |
| HEX8  | 40.3 (10.1%) | 36.9 ( 0.8%) |
| HEX20 | 35.9 (-1.9%) | 36.1 (-1.4%) |
| HEX27 | 35.9 (-1.9%) | 36.1 (-1.4%) |

## Summary

The results for most element types are within about 2% of the target
temperature. However, the lower order elements with the coarse mesh exhibit a
somewhat higher deviation from the target value of about 10%. In practice,
where a mesh refinement study would be performed, this level of inaccuracy of
a solution would be observed and resolved.


!bibtex bibliography
