# Equilibrium reactions: a primer

Notation and definitions are described in [geochemistry_nomenclature.md].

Consider the hypothetical reactions
\begin{equation}
b\mathrm{B} \rightleftharpoons c\mathrm{C} + d\mathrm{D}
\end{equation}
among species B, C and D, where $b$, $c$ and $d$ are the reaction coefficients.  This means that $b$ [moles](geochemistry_nomenclature.md) of B can be created by removing $c$ moles of C and $d$ moles of D, and vice versa.

## The free energy

Now consider the change of free energy, $G$, corresponding to an increase in the number of moles of B, $n_{B}$.  It is
\begin{equation}
\begin{aligned}
\frac{\mathrm{d}G}{\mathrm{d}n_{B}} = & \frac{\partial G}{\partial n_{B}} + \frac{\partial G}{\partial n_{C}}\frac{\partial n_{C}}{\partial n_{B}} + \frac{\partial G}{\partial n_{D}}\frac{\partial n_{D}}{\partial n_{B}} \\
= & \frac{\partial G}{\partial n_{B}} - \frac{\partial G}{\partial n_{C}}\frac{c}{b} - \frac{\partial G}{\partial n_{D}}\frac{d}{b} \ .
\end{aligned}
\end{equation}
The first line expresses that when the number of moles of B is changed, the number of moles of C and D must change too, by the equilibrium reaction (the partial derivatives indicate "keep everything else fixed").  The second line introduces the correct stoichiometry.  That is: an increase in $n_{B}$ must be accompanied by a corresponding decrease in $n_{C}$ and $n_{D}$.

"Equilibrium" means a minimum in the free energy, so the above derivative must be zero.  Also, the total derivatives with respect to $n_{C}$ and $n_{D}$ must be zero, but they give the same result.  The partial derivatives, e.g. $\partial G/\partial n_{B}$, themselves shouldn't be zero, as the system cannot ever evolve along the direction of changing $n_{B}$ while keeping $n_{C}$ and $n_{D}$ fixed.

## Chemical potential

Introduce the chemical potentials
\begin{equation}
\mu_{B} = \frac{\partial G}{\partial n_{B}} \ ,
\end{equation}
etc.  Here, the partial derivative is taken with constant $n_{C}$ and $n_{D}$, as well as constant temperature and pressure.  Then equilibrium reads
\begin{equation}
b\mu_{B} - c\mu_{C} - d\mu_{D} = 0 \ .
\end{equation}
Given the functional forms of the $\mu$, and, say $n_{C}$ and $n_{D}$, we can find $n_{B}$ using this formula.

## Ideal solutions

The "theory of ideal solutions" states that for each constituent
\begin{equation}
\mu = \mu^{0} + RT \log X \ ,
\end{equation}
where

- $\mu$ \[J.mol$^{-1}$\] is the chemical potential
- $\mu^{0}$ \[J.mol$^{-1}$\] is the constant number, independent of temperature, pressure, other constituents, etc, but which depends on the reaction in question
- $R = 8.314\ldots\,$J.K$^{-1}$.mol$^{-1}$ is the gas constant
- $T$ \[K\] is temperature
- $\log$ is the natural logarithm
- $X$ \[dimensionless\] is the [mole fraction](geochemistry_nomenclature.md) of the constituent

## Mass action for ideal solutions

Equilibrium then reads
\begin{equation}
0 = \frac{b\mu_{\mathrm{B}}^{0} - c\mu_{\mathrm{C}}^{0} - d\mu_{\mathrm{D}}^{0}}{RT} + \log \frac{X_{\mathrm{B}}^{b}}{X_{\mathrm{C}}^{c}X_{\mathrm{D}}^{d}} \ .
\end{equation}
The first term involves the reaction's standard free energy, and allows introduction of the so-called equilibrium constant for the reaction, which is defined to be
\begin{equation}
\log K = \frac{b\mu_{\mathrm{B}}^{0} - c\mu_{\mathrm{C}}^{0} - d\mu_{\mathrm{D}}^{0}}{RT}
\end{equation}
Note that the equilibrium "constant" is not constant: it at least depends on temperature.  It is also reaction dependent.

Finally, equilibrium can be stated as
\begin{equation}
K = \frac{X_{\mathrm{C}}^{c}X_{\mathrm{D}}^{d}}{X_{\mathrm{B}}^{b}}
\end{equation}
This is called "mass action".

## Non-ideal solutions

The chemical potential in the non-ideal case is written as
\begin{equation}
\mu = \mu^{0} + RT \log a \ ,
\end{equation}
where most parameters have been defined above, but

- $a$ \[dimensionless\] is the constituent's [activity](activity_coefficients.md), which depends on temperature and potentially other things.

## Mass action for non-ideal solutions

This is
\begin{equation}
K = \frac{a_{\mathrm{C}}^{c}a_{\mathrm{D}}^{d}}{a_{\mathrm{B}}^{b}} \ .
\end{equation}
