# Grain Growth Model

Grain boundaries (GBs) migrate to reduce the total free energy of the system.
Various sources of free energy drive the GB migration, including stored defect
energy, deformation energy, and GB energy. Various modeling approaches have been
applied to model grain boundary migration, though the phase field method has
emerged for one of the more popular. While all of the various models predict
similar behavior, the phase-field method has emerged as the most popular due to
its flexibility and computational efficiency.

The grain growth model implemented in MARMOT is from Moelans et al.
[!cite](moelans_quantitative_2008).  In the model, each grain is represented by a
continuous order parameter $\eta_i$ equal to one within the grain and zero in
all other grains.  The free energy for this problem is
\begin{equation}
  f_{loc} = \mu \left( \sum_i^N \left(\frac{\eta_i^4}{4} - \frac{\eta_i^2}{2} \right)  + \gamma \sum_{i=1}^N \sum_{j>i}^N \eta_i^2 \eta_j^2 \right) + \frac{1}{4}.
\end{equation}

The evolution of each grain's order parameter is defined with the Allen-Cahn
equation.  Specific for the grain growth model, a local energy kernel called
[`ACGrGrPoly`](ACGrGrPoly.md) was created, in which
\begin{equation}
  \frac{\partial f_{loc}}{\partial \eta_i} =  \mu \left( \eta_i^3 - \eta_i  + 2 \gamma \sum_{j=1}^N \eta_i \eta_j^2 \right).
\end{equation}

The code in the kernels takes the form:

```
  SumEtaj += (*_vals[i])[_qp]*(*_vals[i])[_qp];
  return _mu[_qp]*(_u[_qp]*_u[_qp]*_u[_qp] - _u[_qp] + 2.0*_gamma*_u[_qp]*SumEtaj);
```

where `_u[_qp]` stores the current order parameter value $\eta_i$ and
`_vals[i][_qp]` stores the values of $\eta_j$. Since this equation takes the
same form for each order parameter, only one kernel was created and reused for
each order parameter.

The model parameters $L_j$, $\mu$ and $\kappa_j$ are defined in terms of the
grain boundary (GB) surface energy $\sigma$, the diffuse GB width $w_{GB}$ and
the GB mobility $m_{GB}$. These expressions are coded in various MOOSE materials
for various types of physical materials. The most generic form of the equations
is in the material [`GBEvolution`](GBEvolution.md).

The grain growth model can have an arbitrary number of variables, depending on
the number of grains that will be represented.  The residual equation used to
solve for the value of each order parameter at a specific time is divided into
parts and implemented in separate kernels.

The default input file for MOOSE, in which each variable and the three kernels
needed for each variable are added individually, would be very cumbersome for
such a system.  Therefore, we have created a custom input file syntax that
automates the creation of the various order parameters and their kernels.  This
was created with the Action system within MOOSE, and the most basic action that
creates grain growth variables and kernels is
[`GrainGrowthAction`](GrainGrowthAction.md). Given the number of order
parameters (`n_crys`) and some base name to which the numbers will be appended
(`var_name_base`), each variable is created, e.g. if `var_name_base = gr` and
`n_crys = 3`, then the variables `gr0`, `gr1` and `gr2` will be created.

The initial conditions (ICs) for the polycrystal models are created using the
ICs block in the input file. Under the ICs block, there is an additional block
called `PolycrystalICs`. Under this block, there are various options to
create initial conditions for bicrystals, tricrystals and polycrystals. These
options are shown in the Table, below.

| Action name                 | description                                                                                                                   |
| --------------------------- | ----------------------------------------------------------------------------------------------------------------------------- |
| `BicrystalBoundingBoxIC`    | Two grains defined by a bounding box                                                                                          |
| `BicrystalCircleGrainIC`    | Circle grain and matrix grain                                                                                                 |
| `Tricrystal2CircleGrainsIC` | Two circle grains and a matrix grain                                                                                          |
| `PolycrystalHexGrainIC`     | `n_grain` grains in a hexagonal structure                                                                                     |
| `PolycrystalColoringIC`     | `n_grain` grains using a plugin based tesselation (using user objects such as [`PolycrystalVoronoi`](PolycrystalVoronoi.md))  |
| `PolycrystalRandomIC`       | Randomly seeds grain order parameters                                                                                         |

An additional option is to reconstruct the microstructure from experimental data
using the [`EBSD Reader`](ICs/EBSD.md) system. Given data exported from EBSD
software in our specific format, the initial grain structure can mimic the
microstructure of the data.

With the above information, we are ready to run grain growth simulations. The
first step is to verify the model predictions against analytical models of grain
growth. Here, we derive the analytical model for two-grain configurations, a
circular grain imbedded in a larger grain and a half loop grain imbedded in a
larger grain. The grain boundary velocity is calculated according to
\begin{equation}
v = M F,
\end{equation}
where $F$ is the driving force. In our simple verification cases, the driving
force depends only on the curvature of the grain boundary and can be written as
\begin{equation}
F = \frac{\gamma}{R},
\end{equation}
where $\gamma$ is the GB energy and $R$ is the radius of curvature. The change
in area with time is equal to
\begin{equation}
\begin{array}{rl}
\frac{\partial A}{\partial t} &= 2 \pi R \frac{\partial R}{\partial t} = 2 \pi R v\\
&= 2 \pi M \gamma.
\end{array}
\end{equation}
Thus, the area at any time $t$ can be calculated with
\begin{equation}
A = A_0 - 2 \pi M \gamma t,
\end{equation}
where $A_0$ is the initial radius of the circular grain. Following the same
approach for the half loop geometry gives the expression
\begin{equation}
A = A_0 - \pi M \gamma t.
\end{equation}
