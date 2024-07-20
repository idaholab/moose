# 1D Reflection Benchmark

This document describes the 1D metal-backed dielectric slab benchmark / verification
test for the electromagnetics module. This is based on Section 3.4 in [!citep](jin-fem).
The goal of the benchmark is to accurately determine the power reflected by the
slab when impinged by an electric plane wave, compared to an analytic solution.
Below is a summary of the test, along with relevant background theory, results,
and the test input file for review.

## Model Geometry

!style halign=left
The geometry used in this benchmark is shown below in [slab-geometry].

!media slab.png
       style=width:50%;margin:auto;
       id=slab-geometry
       caption=Slab reflection benchmark geometry.

## Governing Equations and Boundary Condition

As described in [!citep](jin-fem), this benchmark models a uniform plane wave
striking an inhomogeneous dielectric backed by a perfectly conducting grounded
plane. The slab, shown in [slab-geometry], has a thickness $L$, a relative
permittivity $\epsilon_r$, and a relative permeability $\mu_r$. Both material
properties are functions of position in the slab. The medium outside the slab is
free space ($\epsilon_r = \mu_r = 1$). An $E_z$-polarized plane wave is the
incoming wave, and can be represented in general by

!equation
E_z^{\text{inc}} (x, y) = E_0 \text{e}^{j k_0 x \text{cos} \theta - j k_0 y \text{sin} \theta}

where

- $E_0$ is the magnitude of the incident field,
- $k_0$ is the wavenumber ($2 \pi / \lambda$) where $\lambda$ is the wavelength),
- $j = \sqrt{-1}$, and
- $\theta$ is the incidence angle of the wave.

The general scalar Helmholtz equation governing the electric field in this
scenario is given by

!equation id=eqn-helmholtz
\frac{\text{d}}{\text{d}x} \left( \frac{1}{\mu_r} \frac{\text{d} E_z}{\text{d}x}\right) + k_0^2 \left( \epsilon_r - \frac{1}{\mu_r} \text{sin}^2 \theta \right) E_z = 0

The boundary condition on the conducting backing plane is given by

!equation id=dirichlet-bc
E_z (0, y) = 0

which is implemented in code by [DirichletBC.md] and the boundary condition just
inside the slab interface with free space is given by

!equation
\left[ \frac{1}{\mu_r} \frac{\text{d} E_z}{\text{d}x} + j k_0 \text{cos} \theta E_z(x) \right]_{x = L_0} = 2j k_0 \text{cos} \theta E_0 \text{e}^{j k_0 L \text{cos} \theta}

which is implemented in code by [EMRobinBC.md].

## Model Parameters and Functions

!style halign=left
The `JinSlabCoeffFunc` test function object (located in `modules/electromagnetics/test/src/functions`)
is used in this benchmark to represent the coefficient shown in [eqn-helmholtz].
The complex-valued function is

!equation
f(x) = \mu_r k_0^2 \left(\epsilon_r - \frac{1}{\mu_r}\sin^2\left(\frac{\theta \: \pi}{180^{\circ}}\right)\right)

The relative electric permittivity in the model is defined as

!equation
\epsilon_r = 4 + (2 - j0.1)\left(1 - \frac{x}{L}\right)^2

and the relative magnetic permeability is defined as

!equation
\mu_r = 2 - j0.1

Other model parameters are shown below in [table-slab-reflection-parameters].

!table id=table-slab-reflection-parameters caption=Constant model parameters for the slab reflection benchmark study.
| Parameter (unit) | Value |
| - | - |
| Wave frequency (MHz) | 20 |
| Wavelength, $\lambda$ (m) | 15 |
| Wavenumber, $k_0$ (1/m) | $2 \pi$ / $15 \approx 0.4189$ |
| Slab thickness, $L = 5\lambda$ (m) | 75 |
| Incident wave magnitude, $E_0$ (V/m) | 1 |

## Analytic Solution for Reflection Coefficient

!style halign=left
The analytic solution approach is outlined in [!citep](jin-fem), Section 3.4.2,
but will be summarized here. First the slab in [slab-geometry] should be divided
into $M$ thin layers as shown in [slab-layers], and the analytic solution to the
original Helmholtz equation in [eqn-helmholtz] in a single layer should be found.

!media slab_layers.png
       style=width:50%;margin:auto;
       id=slab-layers
       caption=Slab reflection benchmark geometry, cut into $M$ layers.

This solution can be shown to be

!equation
E_{zm} = \left( A_m \text{e}^{j k_{x \: m} x}  + B_m \text{e}^{-j k_{x \: m} x}\right) \text{e}^{-j k_0 y \text{sin}\theta}

where $A_m$ and $B_m$ are unknown constant coefficients. Enforcing continuity
across the layer interfaces in the $x$ direction, we can find the wave reflection
coefficient for the interface between the $m$th and $(m+1)$th layer to be

!equation
R_{m+1} = \frac{\eta_{m+1,m} + R_m \text{e}^{-2j k_{x \: m} x_{m+1}}}{1 + \eta_{m+1,m} R_m \text{e}^{-2j k_{x \: m} x_{m+1}}} \text{e}^{2j k_{x \: m+1} x_{m+1}}

where

!equation
R_m = \frac{B_m}{A_m}

!equation
k_{x \: m} = k_0 \sqrt{\mu_{r \:m} \epsilon_{r \: m} - \text{sin}^2 \theta}

!equation
\eta_{m+1, m} = \frac{\mu_{r \: m} k_{x \: m+1} - \mu_{r \: m+1} k_{x \: m}}{\mu_{r \: m} k_{x \: m+1} + \mu_{r \: m+1} k_{x \: m}}

Given the DirichletBC placed on the metal surface in [dirichlet-bc], it can be
found that $R_1 = -1$. Thus, the analytic solution can be determined recursively
for the desired number of layers. The coefficient reported below in
[reflection-results] represents the magnitude of the coefficient $|R_{M+1}|$,
over a range of frequencies on the dielectric interface.

## ReflectionCoefficient object

!style halign=left
The postprocessor object used in this benchmark to calculate the percentage of
reflected power is described in more detail in the [ReflectionCoefficient.md]
documentation. To summarize, the object evaluates the following expression:

!equation
R_{wave} = \frac{E_z(x = L) - E_0 \text{e}^{j k_0 L \text{cos} \theta}}{E_0 \text{e}^{-j k_0 L \text{cos} \theta}}

which assumes $E_z$-polarized plane waves as used in this benchmark. Because wave
power is proportional to the magnitude of the wave squared, the coefficient
reported below in the results section representing the percentage of reflected
power is

!equation
R_{power} = |R_{wave}|^2

## Input File

!listing slab_reflection.i

## Results and Discussion

!style halign=left
The input file shown above was swept over a range of angles from $\theta = 0^\circ$ to
$\theta = 90^\circ$, and the results for 100 elements compared to the analytic
solution are shown below in [reflection-results], where there is good agreement
with the analytical solution. Jin also saw a deviation between the analytic
solution and the finite element solution corresponding to smaller angles of
incidence. They attributed this deviation to the idea that as $\theta$ increases,
the magnitude of the coefficient in the second term of [eqn-helmholtz] decreases.
In physical terms, this means that there will be slower field variation in the
$x$-direction as we approach $\theta = 90^{\circ}$, giving more consistent results
between the two solutions. A 1D model might not be adequate to capture the
reflection exactly with smaller $\theta$.

!media slab_results.png
       style=width:100%;margin:auto;
       id=reflection-results
       caption=Slab reflection benchmark results.
