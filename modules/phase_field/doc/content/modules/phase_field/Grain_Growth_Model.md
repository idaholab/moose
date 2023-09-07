# Grain Growth Model

Grain boundaries (GBs) migrate to reduce the total free energy of the system.
Various sources of free energy drive the GB migration, including stored defect
energy, deformation energy, and GB energy. Various modeling approaches have been
applied to model GB migration. While all of the various methods predict
similar behavior, the phase-field method has emerged as one of the more popular due to its flexibility.

In MOOSE, we have implemented the multiphase grain growth model from [!cite](moelans_quantitative_2008). The variables evolve to reduce the overall free energy of the system, representing GB migration. The evolution of each grain's order parameter is defined with the Allen-Cahn equation:
\begin{equation}
  \frac{\partial \eta_i}{\partial t} = - L \frac{\delta F}{\delta \eta_i},
\end{equation}
where $F$ is the free energy functional, $L$ is the order parameter mobility, and the $\delta$ operator represents a variational derivative. The free energy for this problem is
\begin{equation}
  F = \int_V f_{loc}(\eta_0, \eta_1, \ldots, \eta_N) + f_{add} (\eta_0, \eta_1, \ldots, \eta_N) + \kappa \sum^N_i |\nabla \eta_i|^2,
\end{equation}
where $N$ is the total number of order parameters, $f_{loc}$ represents the local free energy density, and $f_{add}$ represents any additional sources of energy density. For the grain growth model,
\begin{equation}
  f_{loc} = \mu \left( \sum_i^N \left(\frac{\eta_i^4}{4} - \frac{\eta_i^2}{2} \right)  + \gamma \sum_{i=1}^N \sum_{j>i}^N \eta_i^2 \eta_j^2 + \frac{1}{4} \right),
\end{equation}
where $\mu$ is the free energy weight and $\gamma=1.5$ for symmetric interfacial profiles.

The model parameters $L$, $\mu$ and $\kappa$ are defined in terms of the
GB energy $\sigma$, the diffuse GB width $w_{GB}$, and
the GB mobility $M_{GB}$. In reality, the GB energy and mobility are anisotropic, depending on both the misorientation and inclination of the GB. However, it is common to assume that the GB properties are isotropic. In the case of isostropic GB energy and mobility and symmetric interfacial profiles ($\gamma = 1.5$), [!cite](moelans_quantitative_2008) defined expressions for the model parameters in terms of $\sigma$, $w_{GB}$, and $M_{GB}$:
\begin{equation}
L = \frac{4}{3} \frac{M_{GB}}{w_{GB}}
\end{equation}
\begin{equation}
\mu = 6 \frac{\sigma}{w_{GB}}
\end{equation}
\begin{equation}
\kappa = \frac{3}{4} \sigma w_{GB}
\end{equation}

## MOOSE Implementation

The grain growth model implementation in MOOSE can simulate grain growth in 2D and 3D, and can model large polycrystals with thousands of grains. A system of Allen-Cahn equations is solved to define the grain boundary migration. In MOOSE, we solve them in residual weak form:
\begin{equation}
  \mathcal{R}_{\eta_i} = \left( \frac{\partial \eta_i}{\partial t},\psi_m \right) + \left( L \frac{\partial f_{loc}}{\partial \eta_i}, \psi_m \right) - \left(\kappa \nabla \eta_i, L \nabla \psi_m \right)  = 0,
\end{equation}
where $\psi_m$ is the test function. In standard MOOSE fashion, the various terms in the Allen-Cahn equation have been implmented in separate kernels. The MOOSE objects that are used in the grain growth model are summarized, below:

+Materials+

- [`GBEvolution`](GBEvolution.md) - Defines the model parameters $L$, $\mu$, and $\kappa$ using the equations assuming isotropic properties.
- [`GBAnistropy`](phase_field/Grain_Boundary_Anisotropy.md) - Defines parameters $L$, $\mu$, and $\kappa$ considering misorientation dependence for the GB energy.

+Kernels+

- [`TimeDerivative`](/TimeDerivative.md) - Defines the $\left( \frac{\partial \eta_i}{\partial t},\psi_m \right)$ term.
- [`ACGrGrPoly`](/ACGrGrPoly.md) - Defines the $\left( L \frac{\partial f_{loc}}{\partial \eta_i}, \psi_m \right)$ term.
- [`ACInterface`](/ACInterface.md) - Defines the $\left(\kappa \nabla \eta_i, L \nabla \psi_m \right)$ term.

+AuxKernels+

- [`BndsCalcAux`](/BndsCalcAux.md) - To visualize the GB, it is convenient to use $\mathrm{bnds} = \sum_i^N \eta_i^2$.

## Simplified MOOSE syntax

The default input file for MOOSE, in which each variable and kernel
needed for each variable are added individually, would be very cumbersome for
the grain growth model which can have many variables.  Therefore, Actions have been created that
automate the creation of the various order parameters and their kernels.

- [`GrainGrowthAction`](GrainGrowthAction.md) has the simplest syntax. It creates all of the variables and kernels needed for a basic grain growth model.
- [`PolycrystalVariablesAction`](PolycrystalVariablesAction.md) just creates the variables. It can also be used for other models besides grain growth to create a series of variables with standard naming syntax.
- [`PolycrystalKernelAction`](PolycrystalKernelAction.md) just creates the kernels needed for each of the order parameter variables.


## Initial Conditions

The initial conditions (ICs) for the polycrystal models are created using the
ICs block in the input file. Under the ICs block, there is an additional block
called `PolycrystalICs`. Under this block, there are various options to
create initial conditions for bicrystals, tricrystals and polycrystals. These
options are shown in the Table, below.

| IC Action name                 | Description                                                                                                                   |
| --------------------------- | ----------------------------------------------------------------------------------------------------------------------------- |
| [`BicrystalBoundingBoxIC`](BicrystalBoundingBoxICAction.md) | Two grains defined by a bounding box |
| [`BicrystalCircleGrainIC`](BicrystalCircleGrainICAction.md) | Circle grain and matrix grain |
| [`Tricrystal2CircleGrainsIC`](Tricrystal2CircleGrainsICAction.md) | Two circle grains and a matrix grain |
| [`PolycrystalRandomIC`](PolycrystalRandomICAction.md)       | Randomly seeds grain order parameters |
| [`PolycrystalColoringIC`](PolycrystalColoringICAction.md) | [Polycrystal initial conditions](ICs/PolycrystalICs.md). Grain shapes are specified by a polycrystal UserObject (see below). |

The polycrystal UserObjects available for use with [`PolycrystalColoringIC`](PolycrystalColoringICAction.md) are:

- [`PolycrystalVoronoi`](PolycrystalVoronoi.md) - Creates polycrystal grain structure using a Voronoi tesselation.
- [`PolycrystalHex`](PolycrystalHex.md) - Creates a hexagonal polycrystal grain structure.
- [`PolycrystalCircles`](PolycrystalCircles.md) - Creates circular/spherical grains from a file defining center coordinates and radii.
- [`PolycrystalEBSD`](PolycrystalEBSD.md) - Reconstructs grain structures from EBSD data in a specific file format. Used in conjunction with the [`EBSD Reader`](ICs/EBSD.md).

There is also one tricrystal IC without an associated action: [`TricrystalTripleJunctionIC`](TricrystalTripleJunctionIC.md).

## Variable Remapping

If each order parameter is used to represent one grain, the computational cost gets very high to model polycrystal grain growth. However, if each order parameter is used to represent more than one grain, unphysical grain coalescence occurs when grains represented by the same variable come in contact. MOOSE uses the remapping algorithm [GrainTracker](/GrainTracker.md) to overcome this issue. Order parameters are used to represent multiple grains and grains are then remapped to other variables when needed to avoid coalescence.

## Model Verification

The grain growth model has been verified against analytical solutions for two cases, a
circular grain imbedded in a larger grain and a half loop grain imbedded in a
larger grain.

### Circular Grain

For a circular grain, the grain boundary velocity is calculated according to
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
where $A_0$ is the initial radius of the circular grain.

This case can be easily implemented using MOOSE, and the change in the area with time is outputted using a Postprocessor:

!listing modules/phase_field/test/tests/grain_growth/test.i

### Half Loop Grain

Following the same approach from the circle grain but for the half loop geometry gives the expression
\begin{equation}
A = A_0 - \pi M \gamma t.
\end{equation}

This case can also be easily implemented in MOOSE, with the area outputted using a Postprocessor:

!listing modules/phase_field/test/tests/grain_growth/thumb.i

## Example Input Files

An example 2D input file for a polycrystal simulation using `GrainTracker` is available. It should be run on at least 4 processors:

!listing modules/phase_field/examples/grain_growth/grain_growth_2D_graintracker.i

An example 3D input file is also available. It should be run on at least 2 processors:

!listing modules/phase_field/examples/grain_growth/grain_growth_3D.i

There is also an example reconstructing the initial grain structure from EBSD data. It should be run on at least 4 processors:

!listing modules/phase_field/examples/ebsd_reconstruction/IN100-111grn.i
