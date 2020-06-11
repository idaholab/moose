# GeochemistryKineticRate

Defines a kinetic rate for a kinetic species.  The general form of this rate is
\begin{equation}
r = kA[M] \left( \prod_{j}m_{j}^{P_j} \right) \left|1 - \left(Q/K\right)^{\theta}\right|^{\eta} \exp\left( \frac{E_{a}}{R} \left(\frac{1}{T_{0}} - \frac{1}{T}\right)\right) {\mathrm{sgn}}(1 - (Q/K)) \ .
\end{equation}
This is a rather complicated equation, and simple examples are given below.  In this equation:

- $r$ (units: mol.s$^{-1}$) is the rate of the kinetic reaction.  If it is positive then the kinetic species' mass will be decreasing (eg, dissolution).  If it is negative then the kinetic species' mass will be increasing (eg, precipitation).
- $k$ is the intrinsic rate constant.  The product $kA[M]$ has units mol.s$^{-1}$ (assuming the simulation's time units are seconds).  Examples are given below.
- $A$ is either the surface area (units: m$^{2}$) for the kinetic species, or the specific surface area (units: m$^{2}$.g$^{-1}$)
- $[M]$ (units: g) is the mass of the kinetic species.  It is optional.  Examples are given below.
- $j$ is an index denoting a promoting species
- $m_{j}$ is either: mass of solvent water (in kg) if the promoting species is H2O; fugacity of a gas if the promoting species is a gas; activity if the promoting species is either H+ or OH-; mobility, otherwise.
- $Q$ is the [activity product](geochemistry_nomenclature.md) defined by the kinetic species' reaction in the database file
- $K$ is the reaction's equilibrium constant defined in the database file
- $\theta$ and $\eta$ are dimensionless exponents
- $E_{a}$ (units: J.mol$^{-1}$) is the activation energy
- $R=8.314472\,$J.K$^{-1}$.mol$^{-1}$ is the gas constant
- $T_{0}$ (units: K) is a reference temperature
- $T$ (units: K) is the temperature
- ${\mathrm{sgn}}(1-(Q/K))$ is -1 if $Q>K$ (kinetic species mass will increase with time), 1 if $Q<K$ (kinetic species mass will decrease with time).

!alert note
Note that more than one `GeochemistryKineticRate` can be prescribed to a single kinetic species.  The sum of all the individual rates defines the overall rate for each species (see Example 6).  Simply supply your [GeochemicalModelDefinition](GeochemicalModelDefinition.md) with a list of all the rates.

## Example 1

Suppose that the kinetic rate is just a constant:
\begin{equation}
r = k
\end{equation}
for both the forward and backward reactions (ie, for precipitation and dissolution).  Then set:

- $k$ to the constant value, with units mol.s$^{-1}$ (assuming the simulation time is measured in seconds)
- $A=1$ and `multiply_by_mass = false`
- do not use any `promoting_species_names` or `promoting_species_indices`
- set $\theta = 0$ and $\eta = 0$
- set $E_{a} = 0$.

## Example 2

Suppose that the kinetic rate for a certain mineral is proportional the mineral's surface area
\begin{equation}
r = kA
\end{equation}
and that the surface area is known and fixed.  Then set:

- $k$ to the coefficient of proportionality, with units mol.m$^{-2}$.s$^{-1}$ (assuming the simulation time is measured in seconds)
- $A$ to the known mineral surface area, with units m$^{2}$
- `multiply_by_mass = false`
- do not use any `promoting_species_names` or `promoting_species_indices`
- set $\theta = 0$ and $\eta = 0$
- set $E_{a} = 0$.

## Example 3

Suppose that the kinetic rate for a certain mineral is proportional the mineral's free mass
\begin{equation}
r = kM
\end{equation}
Then set

- $k$ to the coefficient of proportionality, with units mol.g$^{-1}$.s$^{-1}$ (assuming the simulation time is measured in seconds)
- $A = 1$
- `multiply_by_mass = true`.  MOOSE will automatically calculate the mineral's free mass depending on its molar mass and the number of free moles.
- do not use any `promoting_species_names` or `promoting_species_indices`
- set $\theta = 0$ and $\eta = 0$
- set $E_{a} = 0$.


## Example 4

Suppose that the kinetic rate for a certain mineral is proportional the mineral surface area
\begin{equation}
r = kA \ ,
\end{equation}
but that the surface area itself isn't known.  Only the mineral's specific surface area (surface area per gram of free mineral m$^{2}$.g$^{-1}$) is known.  Then set

- $k$ to the coefficient of proportionality, with units mol.m$^{-2}$.s$^{-1}$ (assuming the simulation time is measured in seconds)
- $A$ to be the mineral specific surface area, with units m$^{2}$.g$^{-1}$.
- `multiply_by_mass = true`.  MOOSE will automatically calculate the mineral's free mass depending on its molar mass and the number of free moles.
- do not use any `promoting_species_names` or `promoting_species_indices`
- set $\theta = 0$ and $\eta = 0$
- set $E_{a} = 0$.

## Example 5

Suppose that the kinetic rate for a certain redox couple is proportional the quantity of solvent water present:
\begin{equation}
r = kn_{w} \ ,
\end{equation}
Then set

- $k$ to the coefficient of proportionality, with units mol.kg$^{-1}$.s$^{-1}$ (assuming the simulation time is measured in seconds)
- $A = 1$ and `multiply_by_mass = false`.
- `promoting_species_names = H2O` and `promoting_species_indices = 1`
- set $\theta = 0$ and $\eta = 0$
- set $E_{a} = 0$.


## Example 6

Suppose that the kinetic rate for a certain redox couple depends on the pH:
\begin{equation}
r = k_{\mathrm{acid}} a_{H^{+}} + k_{\mathrm{base}} a_{OH^{-}}^{1.5}  \ .
\end{equation}
For this, two `GeochemistryKineticRate` UserObjects must be created.  Each has $A=1$, `multiply_by_mass = false`, $\theta = 0$, $\eta = 0$ and $E_{a}=0$.

- The first has $k = k_{\mathrm{acid}}$, `promoting_species_names = H+` and `promoting_species_indices = 1`.
- The second has $k = k_{\mathrm{base}}$, `promoting_species_names = OH-` and `promoting_species_indices = 1.5`.

These are then supplied to the [GeochemicalModelDefinition](GeochemicalModelDefinition.md) using its `kinetic_rate_descriptions` input.


## Example 7

Suppose that the kinetic rate for a certain redox couple depends on the pH, the molality of Ca$^{2+}$ and the temperature:
\begin{equation}
r = k_{\mathrm{acid}} a_{H^{+}}^{1.5} m_{Ca++}^{0.3}\exp(E_{a}/(RT)) \ .
\end{equation}
Then set

- $k = k_{\mathrm{acid}}$
- $A= 1$ and `multiply_by_mass = false`
- promoting_species_names = "H+ Ca++" and  and promoting_species_indices = "1.5 0.3"
- $E_{a}$
- `one_over_T0 = 0`



!syntax parameters /UserObjects/GeochemistryKineticRate

!syntax inputs /UserObjects/GeochemistryKineticRate

!syntax children /UserObjects/GeochemistryKineticRate
