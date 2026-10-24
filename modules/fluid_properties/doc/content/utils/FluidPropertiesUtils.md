# Utilities for the Fluid Properties Module

## Newton method solver

The fluid properties module includes a 1D and a 2D Newton method solver. 

### 1D solver

The 1D Newton method solver is used to solve the problem:

- Find $z$ such that $f(x, z) = y$, with $x$, $y$ known constants and $f$ a function. 


$f$ should compute both its return value and the derivatives of its value with regards to
both its variables using references.

The routine will error if a NaN is encountered or if the maximum number of iterations is exceeded.

### 2D solver

The 2D Newton method solver is used to solve the problem:

\begin{equation}
\begin{split}
f1(x, y) &= a \\
f2(x, y) &= b
\end{split}
\end{equation}

with $a$, $b$ known constants, $f1$, $f2$ functions and $x$, $y$ the real-valued variables to solve for.


The functions should compute both their return value and the derivatives of their value with regards to
both variables using references.

The routine will +not+ error if a NaN is encountered or if the maximum number of iterations is exceeded.
Instead, a `converged` boolean will be set to false, and the routine will return the last valid
estimates of $x$ and $y$. It is the responsibility of the user to handle this unconverged output.

## Brent's method solver

Brent's method may be used to find roots of a general function.
It was adapted from `Numerical Recipes in C++`.

More information may be found [here](https://en.wikipedia.org/wiki/Brent%27s_method).

## NASA 7-coefficient polynomials id=nasa7-polynomials

The `NASA7` utilities, as used in [NASA7IdealGasFluidProperties.md], evaluate the standard-state
thermodynamic properties of ideal-gas species from NASA 7-coefficient polynomials
[!citep](mcbride1993). Each species has two sets of
coefficients $a_0, \ldots, a_6$, one for temperatures $T \le T_\text{mid}$ and one for
$T > T_\text{mid}$:

!equation
\frac{c_p^\circ}{R} = a_0 + a_1 T + a_2 T^2 + a_3 T^3 + a_4 T^4 \,,

!equation
\frac{h^\circ}{R T} = a_0 + \frac{a_1}{2} T + \frac{a_2}{3} T^2 + \frac{a_3}{4} T^3
  + \frac{a_4}{5} T^4 + \frac{a_5}{T} \,,

!equation
\frac{s^\circ}{R} = a_0 \ln T + a_1 T + \frac{a_2}{2} T^2 + \frac{a_3}{3} T^3
  + \frac{a_4}{4} T^4 + a_6 \,,

where $c_p^\circ$, $h^\circ$, and $s^\circ$ are the molar isobaric heat capacity, enthalpy, and
entropy at the reference pressure of the data, and $R$ is the universal gas constant. The
enthalpy includes the enthalpy of formation, so the enthalpies of different species share a
common reference and reaction enthalpies follow from their differences. The entropy is the
absolute (third-law) entropy. The utilities return these dimensionless forms, so they do not
depend on the value of $R$, and they accept automatic differentiation types.

The built-in species are H$_2$, O$_2$, H$_2$O, CH$_4$, CO, CO$_2$, and N$_2$, with the
GRI-Mech 3.0 data [!citep](smith_grimech30) as distributed with Cantera 3.2.0
[!citep](cantera320) in `gri30.yaml`. The reference pressure is 101325 Pa, and
$T_\text{mid} = 1000$ K. The fits cover 200 K to 3500 K, except for N$_2$, which covers 300 K to
5000 K. The utilities do not check these ranges. The molar masses are those Cantera computes from
its atomic weights.

At $T_\text{mid}$ the low-temperature polynomial applies, as in Cantera. The two polynomials of
a species do not match exactly at $T_\text{mid}$: for the built-in species, the mismatch is below
$4 \times 10^{-7}$ relative in $c_p^\circ$, $6 \times 10^{-3}$ J/mol in $h^\circ$, and
$2 \times 10^{-5}$ J/(mol K) in $s^\circ$.

## Gas transport properties id=gas-transport

The `GasTransport` utilities, as used in [NASA7IdealGasFluidProperties.md] and
[IdealGasMixtureFluidProperties.md], evaluate transport properties of dilute gases from kinetic
theory [!citep](poling2001). The Chapman-Enskog viscosity of a species and binary diffusion
coefficient of a pair of species $i$ and $j$ are

!equation
\mu = \frac{5}{16} \frac{\sqrt{\pi m k_B T}}{\pi \sigma^2 \Omega^{(2,2)*}(T^*)} \,, \qquad
D_{ij} = \frac{3}{16} \sqrt{\frac{2 \pi k_B T}{m_{ij}}} \,
  \frac{k_B T}{p \pi \sigma_{ij}^2 \Omega^{(1,1)*}(T^*_{ij})} \,,

where $m$ is the molecular mass, $m_{ij} = m_i m_j / (m_i + m_j)$ is the reduced mass of the pair,
$k_B$ is the Boltzmann constant, $\sigma$ is the collision diameter, $T^* = k_B T / \epsilon$ is the
reduced temperature with the depth $\epsilon$ of the Lennard-Jones potential, and the pair parameters
follow the combining rules $\sigma_{ij} = (\sigma_i + \sigma_j)/2$ and
$\epsilon_{ij} = \sqrt{\epsilon_i \epsilon_j}$. The reduced collision integrals
$\Omega^{(1,1)*}$ and $\Omega^{(2,2)*}$ of the Lennard-Jones (12-6) potential follow the fits of
[!citet](neufeld1972), valid for $0.3 \le T^* \le 100$.

The Lennard-Jones parameters of the built-in species, H$_2$, O$_2$, H$_2$O, CH$_4$, CO, CO$_2$, and
N$_2$, are those of GRI-Mech 3.0 [!citep](smith_grimech30) as distributed with Cantera 3.2.0
[!citep](cantera320), the same source as the NASA 7-coefficient data. Their reduced temperatures
remain within the range of the fits over the temperature ranges of the NASA 7-coefficient data.
The viscosities of the species other than H$_2$O agree with those of Cantera within 0.5%. The
GRI-Mech parameters of H$_2$O assume a correction for its dipole moment, which is not applied here,
so the binary diffusion coefficients of pairs with H$_2$O differ from those of Cantera by up to 4%.

The thermal conductivity of a nonpolar gas follows from its viscosity $\mu$ and molar heat capacity
$c_p$ with the modified Eucken correlation [!citep](poling2001),

!equation
k = \mu R_s \left( 1.32 \, \frac{c_p}{R} + 0.45 \right) ,

where $R_s$ is the specific gas constant and $R$ the universal gas constant.

The viscosity of steam in the limit of zero density follows the formulation of [!citet](iapws2008),
as in [Water97FluidProperties.md], and its thermal conductivity in the limit of zero density that of
[!citet](iapws2011). [Water97FluidProperties.md] uses the formulation of [!citet](iaps1985) for the
thermal conductivity, whose zero-density term agrees within 2% below 1500 K but decreases above
about 2200 K and becomes negative near 3300 K. Both formulations are fitted up to about 1173 K.

!bibtex bibliography