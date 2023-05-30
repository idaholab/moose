# Evanescent Wave Decay Benchmark

This document describes the evanescent wave decay benchmark / validation test
for the electromagnetics module. Below is a summary of the test, along with
relevant background theory, results, and the test input file for review.

## Model Geometry

!style halign=left
The domain geometry for this case is shown below:

!media waveguide_discontinuity_geometry.png
       style=width:100%;margin:auto;
       id=waveguide-geometry
       caption=Evanescent wave decay benchmark geometry.

Note that the 0.5 cm wide region to the left of the dashed line is the location
of a volumetric current source used to excite the propagating wave.

## Governing Equations and Boundary Conditions

!style halign=left
In this simulation, both the real and imaginary components of the electric field
wave will be simulated separately as vector finite element variables (with the
magnitude displayed in [field-results-20] and [field-results-284]), but the
frequency-domain electric field wave equation in general is given by:

!equation
\nabla \times \nabla \times \vec{E} - \mu_0 \varepsilon \omega^2 \vec{E} = -j \mu_0 \omega \vec{J}

where

- $\vec{E}$ is the complex electric field vector in V/m,
- $\mu_0$ is the vacuum magnetic permeability of the medium in H/m,
- $\varepsilon$ is the electric permittivity of the medium in F/m,
- $\omega$ is the operating frequency in rad/s
- $\vec{J}$ is the external current density in A/$\text{m}^2$, and
- $j$ is the imaginary unit ($j^2 = -1$).

In the EM module, these terms are represented by the [CurlCurlField.md],
[VectorFunctionReaction.md], and [VectorCurrentSource.md] objects, respectively. The
current source in this study is applied in the $y$-direction and only on the real
component, so in 2D we have

\begin{equation}
\begin{aligned}
\vec{J} &= (J_{x,R} + j J_{x,I})\:\hat{\mathbf{i}} + (J_{y,R} + j J_{y,I})\:\hat{\mathbf{j}} \\
&= J_{y,R} \: \hat{\mathbf{j}}
\end{aligned}
\end{equation}

where

- $J_{i,R}$ is the real current density component in the $i$ direction,
- $J_{i,I}$ is the imaginary current density component in the $i$ direction, and
- $\hat{\mathbf{i}}$ and $\hat{\mathbf{j}}$ are Cartesian unit vectors in the $x$ and $y$ directions, respectively.

At the entry and exit ports, this model uses the [VectorEMRobinBC.md] object. This is given by

!equation
\hat{\mathbf{n}} \times \left( \frac{1}{\mu_0} \nabla \times \vec{E} \right) + j\beta(\mathbf{r}) \hat{\mathbf{n}} \times \left( \hat{\mathbf{n}} \times \vec{E} \right) = \hat{\mathbf{n}} \times (\nabla \times \vec{E}_{inc}) + j\beta(\mathbf{r}) \hat{\mathbf{n}} \times \left( \hat{\mathbf{n}} \times \vec{E}_{inc} \right)

where

- $\vec{E}_{inc}$ is the incoming electric field vector (set to zero since the wave is excited by a current source),
- $\beta(\mathbf{r})$ is a function containing the condition coefficients (in this case, the wave number of the excited wave $k_0$), and
- $\hat{\mathbf{n}}$ is the boundary normal vector.

## Model Parameters

!style halign=left
Important constant model parameters are shown below in [evanescent-model-parameters]. Note the following:

- The electric permittivity mentioned above is related to the relative permittivity mentioned below using the relation $\varepsilon = \varepsilon_r \varepsilon_0$ where $\varepsilon_0$ is the vacuum electric permittivity.
- The operating frequency $\omega$ is related to $f$ below using the relation $\omega = 2 \pi f$.

!table id=evanescent-model-parameters caption=Constant model parameters for the evanescent wave decay benchmark study.
| Parameter (unit) | Value(s) |
| - | - |
| Current Source Magnitude, $|\vec{J}|$ (A/$\text{m}^2$) | 1 |
| Operating frequency, $f$ (GHz) | 16 - 28.4 |
| Relative permittivity, $\varepsilon_r$ (-) | 1 |
| Vacuum electric permittivity, $\varepsilon_0$ (F/m) | $8.854 \times 10^{-12}$ |
| Vacuum magnetic permeability, $\mu_0$ (H/m) | $4 \pi \times 10^{-7}$ |

Calculated parameters needed for the boundary conditions (the wave number in the
direction of propagation of the traveling wave in a 2D waveguide) were calculated
using the following equation (more information in [!citep](griffiths-intro)):

!equation
\beta(\mathbf{r}) = k_0 = \frac{\omega}{c}

where:

- $c$ is the speed of light (or $3 \times 10^8$ m/s).

Using the 20 GHz operating frequency presented in [field-results-20], the value
for $\beta$ used in the input file shown below is

!equation
\beta = \frac{2 \pi (20 \times 10^9)}{3 \times 10^8}

## Mesh

!style halign=left
The mesh used in this study was created in [Gmsh], using the geometry shown
above in [waveguide-geometry]. The `.geo` file used to create this mesh is shown
at the end of this section. To reproduce the corresponding `.msh` file in a
terminal, ensure that gmsh is installed and available in the system PATH and simply
run the following command at the location of `waveguide_discontinuous.geo`:

```
gmsh -2 waveguide_discontinuous.geo -clscale 0.004 -order 2 -algo del2d
```

As of Gmsh 4.6.0, this command produces a second order 2D mesh with an element
size factor of 0.004 (not including the point-wise scaling factors contained
within the mesh file) using a Delaunay algorithm. The unstructured, triangular
mesh contains 67028 nodes and 33822 elements. An image of the result is shown
in [evanescent-mesh]. Note that the second order mesh is required because we are
using `NEDELEC_ONE` vector finite elements for all solution variables.

!media waveguide_discontinuity_mesh.png
       style=width:100%;margin:auto;
       id=evanescent-mesh
       caption=Mesh used in the evanescent wave decay benchmark study.

### Mesh File

!listing waveguide_discontinuous.geo

## Evanescent Wave Decay Theory

!style halign=left
If a wave propagating within a waveguide reaches a point where it can no longer
propagate (such as in the case of a discontinuity), then the majority of the wave
energy will reflect and proceed to propagate in the opposite direction. Some
small fraction is transmitted, however, and then decays since no wave can form.
The characteristic evanescent wave decay constant $\lambda$ is given by

!equation
\lambda = \frac{\sqrt{\omega_c^2 - \omega^2}}{c}

The cutoff frequency $\omega_c$ is the *lowest* frequency at which a wave can propagate in
a given geometry. For this 2D case, that is given by

!equation
\omega_c = \frac{\pi c}{a}

In this benchmark, the decay constant of the wave in the "cutoff" region was
calculated and compared to the theory above, given the geometry of the waveguide
being modeled. For more information on cutoff frequency and evanescent wave
decay, please see [!citep](griffiths-intro).

## Input File

!listing evanescent_wave.i

## Solution and Discussion

!style halign=left
Results from the input file shown above compared to the theory over a range of
frequencies are shown below in [evanescent-wave-results]. The decay constants
shown in the plot were determined via an exponential fit of the electric field
magnitude behavior taken down the center of the narrower portion of the waveguide
($y = 0.0075$). Note that the results are in excellent agreement with theory
overall, with the relative error having an average value of 2.7% and reaching a
maximum of ~10.6% by 28.4 GHz. This increasing deviation could be due to the mesh
resolution compared to the wavelength of the traveling wave. At 16 GHz, there are
approximately 4.69 elements per wavelength (considering our mesh element factor
of 0.004 m). At 28.4 GHz, there are only approximately 2.64 elements per wavelength,
potentially leading to an overall loss in fidelity due to poor resolution.

However, increasing the wavelength resolution for the 28.4 GHz case to approximately
the same level as in the 16GHz case doesn't lead to a corresponding decrease in
relative error. What else could be going on here? Returning to the idea of a
cutoff frequency, $f_c$ (where $\omega_c = 2 \pi f_c$) for the narrower section
of the waveguide is ~30 GHz. As the operating frequency approaches the cutoff
frequency, attenuation of the wave is lessened as shown in [field-results-284],
which leads to a slight deviation from the ideal decay rate shown here.

!media evanescent_decay_results.png
       style=width:80%;margin:auto;
       id=evanescent-wave-results
       caption=Results of the evanescent wave decay benchmark study.

Field results at the 20GHz operating frequency outlined earlier are shown below
in [field-results-20]. The interference pattern shown in the left region is due
to the wave reflection after reaching the discontinuity. Also note that the
singularity seen at the sharp reentrant corner is a byproduct of the numerical
model used, and isn't necessarily physical (though some component of that sort
of field enhancement might be - it is a well known issue to separate out the two
effects). The surface normal is ill-defined at the corner node, and thus the
boundary condition suggests that the current must change direction instantaneously
with the modeled conducting surface. This results in a possibly infinite electric
field magnitude. Note that all color bars in the presented results are scaled
for clarity due to the presence of the singularity. Maximum field magnitude at
the singularity is noted on each figure.

Rounding the corner is a common way to try to avoid this issue (as mentioned in
[!citep](jin-fem) and others), and the impact of a round corner with radius of
10 $\mu \text{m}$ (seen in [rounded-geometry]) is shown in [rounded-corner].
Because the rounded corner is not perfectly round, but made up of tiny line
segments, there will still be some opportunities for field singularities to occur.
Indeed, the peak electric field local to the singularity location is higher than
in [field-results-20]; however, the impact of the singularity on the surrounding
field calculation is reduced. Regardless of the geometry adjustment, this suggests
the impact of numerical singularities on the calculated global electric field
magnitude are extremely local (made even more so due to the level of local
refinement used here) due to the nature of the finite element method (minimizing
global error while possibly allowing local ones). Away from this singularity,
this property allows us to successfully use this model to examine evanescent wave
decay in the smaller waveguide region on the right.

!media evanescent_field_results_20.png
       style=width:100%;margin:auto;
       id=field-results-20
       caption=Field results of the evanescent wave decay benchmark study at 20GHz.

!media evanescent_field_results_284.png
       style=width:100%;margin:auto;
       id=field-results-284
       caption=Field results of the evanescent wave decay benchmark study at 28.4GHz. Note the increased electric field strength in the narrower section compared to 20GHz.

!media rounded_corner_results.png
       style=width:100%;margin:auto;
       id=rounded-corner
       caption=Field results of the evanescent wave decay benchmark study at 20GHz, using a rounded corner instead of a sharp one.

!media rounded_mesh.png
       style=width:50%;margin:auto;
       id=rounded-geometry
       caption=Rounded corner in waveguide geometry.
