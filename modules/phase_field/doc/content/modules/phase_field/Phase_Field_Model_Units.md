# Phase Field Model Units

While phase field models are often nondimensionalized, the inherent equations are dimensional. Thus, it is critical to understand and correctly implement the units, especially when coupling multiple physics.

## Units Analysis of Phase Field Equations

In the unit analysis, we break the equations down by units, with the following conventions:

- $l$ - length
- $t$ - time
- $e$ - energy
- $mol$ - moles

### Free Energy Functional

The driving force for evolution in the phase field method is the minimization of a free energy functional. The free energy functional is written as

\begin{equation}
	F = \int_V \big[ f_{loc}(c_1, \ldots,c_N, \eta_1, \ldots, \eta_M) + f_{gr}(c_1, \ldots,c_N, \eta_1,  \ldots, \eta_M) + E_{d} \big] \, dV,
\end{equation}

where $f_{loc}$ defines the local free energy density with the units $e/l^3$ and is a function of concentration variables $c_i$ (typically a molar fraction)
and order parameters $\eta_i$. The gradient energy density

\begin{equation}
	f_{gr} = \sum_i^N \frac{\kappa_i}{2} |\nabla c_i|^2 + \sum^M_j \frac{\kappa_j}{2} |\nabla \eta_j|^2
\end{equation}

where $\kappa_i$ and $\kappa_j$ are gradient energy coefficients with the units $e/l$.  Finally, $E_d$ describes any additional sources of energy in the system, such as deformation or electrostatic energy, with units of $e/l^3$.

### Allen-Cahn Equation

The Allen-Cahn equation after the variational derivative takes the form

\begin{equation}
	\frac{\partial \eta_j}{\partial t} = - L \left( \frac{\partial f_{loc}}{\partial \eta_j} +  \frac{\partial E_{d}}{\partial \eta_j} - \kappa_j \nabla^2 \eta_j \right).
\end{equation}

The units of this equation are

\begin{equation}
	\frac{1}{t} = - \frac{l^3}{e t} \left( \frac{e}{l^3} +  \frac{e}{l^3} - \frac{e}{l} \frac{1}{l^2} \right).
\end{equation}

where the units of $L$ are $l^3/(e t)$.

### Cahn-Hilliard Equation

The Cahn-Hilliard equation after the variational derivative takes the form

\begin{equation}
	\frac{\partial c_i}{\partial t} = V_m \nabla \cdot M_i \nabla \left( \frac{\partial f_{loc}}{\partial c_i} + \frac{\partial E_{d}}{\partial c_i} - \kappa_i \nabla^2 c_i  \right)
\end{equation}

where $V_m$ is the molar volume of the reference state of the material with units of $l^3/mol$. The units of this equation are

\begin{equation}
	\frac{1}{t} = \frac{l^3}{mol} \frac{1}{l} \frac{l^2 mol}{t e} \frac{1}{l} \left( \frac{e}{l^3} + \frac{e}{l^3} - \frac{e}{l} \frac{1}{l^2}  \right)
\end{equation}

where the units of $M_i$ are $(l^2 mol)/(e t)$. Note that some models include the $V_m$ in the mobility term, such that it has units of $l^5/(t e)$.

### CALPHAD Local Free Energies

A common approach for phase field models is to use free energies for the individual phases taken from the CALPHAD approach. CALPHAD free energies $F_\alpha$ typically have units of $e/mol$, therefore they must be converted to free energy densities using the molar volume according to

\begin{equation}
f_\alpha = \frac{F_\alpha}{V_m}.
\end{equation}

Note that when using a multiphase model, the CALPHAD free energies of each phase must be converted with the same reference molar volume $V_m$, with changes in volume between phases handled by stress-free strains.

### Handling Units in MOOSE

There is no inherent unit system in MOOSE. Thus, the units of the phase field equations are set by the user when they define a model. Specifically, the units are set by the local free energy density and the $\kappa$ and mobility parameters ($L$ and $M$). The units in all these terms must be consistent. Additional energy sources, such as the elastic energy, also must have consistent units. In the phase field module, all of these values are created using Material objects. Thus, the units of your system are not set by the kernels but rather by the materials.

One useful practice is to create your material objects to take SI units as input parameters. Then use `length_scale`, `time_scale`, and `energy_scale` input parameters to convert the actual units of the problem. As an example of this, see the [PFParamsPolyFreeEnergy](/PFParamsPolyFreeEnergy.md) material, where the input file block looks like

```yaml
[./Copper]
  type = PFParamsPolyFreeEnergy
  block = 0
  c = c
  T = 1000 # K
  int_width = 30.0
  length_scale = 1.0e-9
  time_scale = 1.0e-9
  D0 = 3.1e-5 # m^2/s, from Brown1980
  Em = 0.71 # in eV, from Balluffi1978 Table 2
  Ef = 1.28 # in eV, from Balluffi1978 Table 2
  surface_energy = 0.7 # J/m^2
[../]
```

*Taken from `phase_field/tests/PolynomialFreeEnergy/split_order4_test.i`*
