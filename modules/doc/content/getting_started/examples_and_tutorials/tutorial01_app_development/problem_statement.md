!content pagination previous=tutorial01_app_development/preface.md
                    next=tutorial01_app_development/step01_moose_app.md
                    margin-bottom=0px

# Problem Statement

Consider a system containing two pressure vessels at differing temperatures as in the experiments presented by [!cite](pamuk2012friction). The vessels are connected via a pipe that contains a filter consisting of close-packed steel spheres as shown in [problem-schematic]. Predict the velocity and temperature of the fluid inside the filter. The pipe is 0.304 m in length and 0.0514 m in diameter. The fluid inside the system is liquid water at approximately 30 degrees Celcius. Each of the steel spheres are 1 mm in diameter.

!media tutorial01_app_development/problem_schematic.png
       style=width:80%;margin-left:auto;margin-right:auto;
       id=problem-schematic
       caption=Schematic of the pressure vessel system for which a custom MOOSE-based application will be designed to solve [!cite](pamuk2012friction).

For this tutorial, the outlined portion of the pipe of length $L$, shown in [problem-schematic], is of particular interest. This region shall serve as the problem domain $\Omega$.

## Governing Equations id=equations

To solve this problem, the following physics must be considered:

- Conservation of Mass:

!equation id=mass
\nabla \cdot \vec{u} = 0

- Conservation of Energy:

!equation id=energy
C \left(\dfrac{\partial T}{\partial t} + \epsilon \vec{u} \cdot \nabla T \right) - \nabla \cdot k \nabla T = 0

- Darcy's Law:

!equation id=darcy
\vec{u} = -\dfrac{\mathbf{K}}{\mu} \left(\nabla p - \rho \vec{g} \right)

The variables shown in [!eqref](mass), [!eqref](energy), and [!eqref](darcy) denote the properties listed in [variables].

!table id=variables caption=Disambiguation of problem variables.
| Symbol | Property |
| - | - |
| $\vec{u}$ | Velocity (Volumetric Flux) Vector |
| $C$ | Heat Capacity |
| $T$ | Temperature |
| $t$ | Time |
| $\epsilon$ | Porosity |
| $k$ | Thermal Conductivity |
| $\mathbf{K}$ | Permeability Tensor |
| $\mu$ | Dynamic Viscosity |
| $p$ | Pressure |
| $\rho$ | Density |
| $\vec{g}$ | Gravity Vector |

If a zero-gravity condition is assumed, i.e., if $\vec{g} = 0$, and if the divergence-free condition of [!eqref](mass) is imposed onto [!eqref](darcy),
then it follows that the two unknowns, $p$ and $T$, must satisfy the following system of two [!ac](PDEs):

!equation id=system
-\nabla \cdot \dfrac{\mathbf{K}}{\mu} \nabla p = 0
\newline \, \newline
C \left(\dfrac{\partial T}{\partial t} + \epsilon \vec{u} \cdot \nabla T \right) - \nabla \cdot k \nabla T = 0

The system overall heat capacity, density, and thermal conductivity are weighted by the contributions of the individual materials and each depend on the porosity of the packed steel sphere medium. These three relationships are defined by the following:

!equation id=heat-cap
C \equiv \epsilon \rho_{f} c_{p,f} + (1 - \epsilon) \rho_{s} c_{p,s}

!equation id=density
\rho \equiv \epsilon \rho_{f} + (1 - \epsilon) \rho_{s}

!equation id=thermal-cond
k \equiv \epsilon k_{f} + (1 - \epsilon) k_{s}

Here, $c_{p}$ denotes specific heat and the subscripts $f$ and $s$ refer to the fluid material (water) and solid material (steel), respectively.

## Material Properties id=mats

The material properties of the fluid ($f$) and the solid ($s$) are given in [mats]. The permeability of the packed steel sphere medium that is present throughout the pipe is assumed to be isotropic. [!cite](pamuk2012friction) provides the following relationship:

!equation id=permeability
K(d) = \frac{1}{2} \begin{bmatrix} -d + 3 & d - 1 \end{bmatrix} \begin{Bmatrix} 0.8451 \\ 8.968 \end{Bmatrix} \times 10^{-9}, \enspace \forall \, d \in [1, 3]

where $d$ is the diameter ($\textrm{mm}$) of the spheres and $K$ denotes the scalar permeability ($\textrm{m}^{2}$), which is a linear function of $d$.

!!!
TODO: The properties listed here are somewhat inconsistent with what we input in the code. First off,
this is not a comprehensive list of all the mat props we'll see in the tutorial. And second, some of
these properties depend on the temperature, for which we make different assumptions about its value
at different steps in the tutorial. I'll need to update this table once I am clear about what our
assumptions are...
!!!

!table id=mats caption=Material property values.
| Property | Value | Units |
| :- | :- | :- |
| Viscosity of water, $\mu_{f}$ | $7.98\times10^{-4}$ |  $\textrm{Pa}\cdot\textrm{s}$ |
| Steel sphere diameter, $d_{s}$ | $1$ | $\textrm{mm}$ |
| Density of water, $\rho_f$ | 996 | $\textrm{kg}/\textrm{m}^3$ |
| Density of steel, $\rho_s$ | 8000 | $\textrm{kg}/\textrm{m}^3$ |
| Thermal conductivity of water, $k_f$ | 0.600 | $\textrm{W}/\textrm{m}\,\textrm{K}$ |
| Thermal conductivity of steel, $k_s$ | 18.00 | $\textrm{W}/\textrm{m}\,\textrm{K}$ |
| Specific heat capacity of water, $c_{p,f}$ | 4180 | $\textrm{J}/(\textrm{kg}\,\textrm{K})$ |
| Specific heat capacity of steel, $c_{p,s}$ | 466 | $\textrm{J}/(\textrm{kg}\,\textrm{K})$ |

!content pagination previous=tutorial01_app_development/preface.md
                    next=tutorial01_app_development/step01_moose_app.md
