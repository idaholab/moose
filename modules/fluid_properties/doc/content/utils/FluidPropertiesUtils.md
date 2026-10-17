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

## NASA 7-coefficient polynomials

The `NASA7` utilities evaluate the standard-state thermodynamic properties of ideal-gas species
from NASA 7-coefficient polynomials [!citep](mcbride1993). Each species has two sets of
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

!bibtex bibliography