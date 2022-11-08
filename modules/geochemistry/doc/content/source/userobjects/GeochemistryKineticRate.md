# GeochemistryKineticRate

This `UserObject` defines a kinetic rate for a kinetic species.  Usually the reaction for a kinetic species $A_{\bar{k}}$ is written:
\begin{equation}
\label{gen_kin_react_eqn}
A_{\bar{k}} \rightleftharpoons \nu_{w\bar{k}}A_{w} + \sum_{i}\nu_{i\bar{k}}A_{i} + \sum_{k}\nu_{k\bar{k}}A_{k} + \sum_{m}\nu_{m\bar{k}}A_{m} + \nu_{p\bar{k}}A_{p} \ ,
\end{equation}
with equilibrium constant $K_{\bar{k}}$, and reaction rate $r$.  Further information can be found on the [theory](geochemistry/theory/index.md) page.

The general form of the rate is
\begin{equation}
r = kA[M] \frac{\tilde{m}^{\tilde{P}}}{(\tilde{m}^{\tilde{P}} + \tilde{K}^{\tilde{P}})^{\tilde{\beta}}}\left( \prod_{\alpha}\frac{m_{\alpha}^{P_{\alpha}}}{(m_{\alpha}^{P_{\alpha}} + K_{\alpha}^{P_{\alpha}})^{\beta_{\alpha}}} \right) \left|1 - \left(Q/K\right)^{\theta}\right|^{\eta} \exp\left( \frac{E_{a}}{R} \left(\frac{1}{T_{0}} - \frac{1}{T}\right)\right) D(Q/K) \ .
\end{equation}
This is a rather complicated equation, and simple examples are given below.  In this equation:

- $r$ (units: mol.s$^{-1}$) is the rate of the kinetic reaction.  If it is positive then the kinetic species' mass will be decreasing (eg, dissolution).  If it is negative then the kinetic species' mass will be increasing (eg, precipitation).
- $k$ is the intrinsic rate constant, which is positive (except in the `raw` and `death` cases mentioned below).  The product $kA[M]$ has units mol.s$^{-1}$ (assuming the simulation's time units are seconds).  Note that mole units are used, even if the initial amount of the kinetic species is given in mass or volume units.  Examples of $k$ are given below.
- $A$ is either the surface area (units: m$^{2}$) for the kinetic species, or the specific surface area (units: m$^{2}$.g$^{-1}$)
- $[M]$ (units: g) is the mass of the kinetic species.  It is optional (hence the square brackets).  Examples are given below.
- $\tilde{m}$ (units: mol.kg$^{-1}$) is the molality of the kinetic species.  Hence $\tilde{m}^{\tilde{P}}/(\tilde{m}^{\tilde{P}} + \tilde{K}^{\tilde{P}})^{\tilde{\beta}}$ is a monod form, where $\tilde{P}$ and $\tilde{\beta}$ are dimensionless parameters, and $\tilde{K}$ is a half saturation (units: mol.kg$^{-1}$).  This factor is optional, and may be omitted by ensuring $\tilde{P} = 0 = \tilde{\beta}$.
- $\alpha$ is a label denoting a promoting species
- $m_{\alpha}$ is either: mass of solvent water (in kg) if the promoting species is H$_{2}$O; fugacity of a gas if the promoting species is a gas; activity if the promoting species is either H$^{+}$ or OH$^{-}$; mobility, otherwise.
- $P_{\alpha}$ is a dimensionless power
- $\beta_{\alpha}$ is a dimensionless power.  The denominator of monod form, $(m_{\alpha}^{P_{\alpha}} + K^{P_{\alpha}})^{\beta_{\alpha}}$, may be omitted by simply setting $\beta_{\alpha} = 0$.
- $K_{\alpha}$ (units: mol.kg$^{-1}$) is a half-saturation constant.
- $Q$ is the [activity product](geochemistry_nomenclature.md) defined by the kinetic species' reaction in the database file
- $K$ is the reaction's equilibrium constant defined in the database file.  This may be modified if the `GeochemistryKineticRate` object is used to model biologically-catalysed reactions.  When a microbe catalyses one mole of the reaction, it captures $E_{c}$ Joules of energy (units: J).  This means $K = K_{\mathrm{database}}\exp( - E_{c}/(RT))$.
- $\theta$ and $\eta$ are dimensionless exponents.  If $\eta = 0$ then $\left|1 - \left(Q/K\right)^{\theta}\right|^{\eta} = 1$ irrespective of the value of $Q$, $K$ and $\theta$.
- $E_{a}$ (units: J.mol$^{-1}$) is the activation energy
- $R=8.314472\,$J.K$^{-1}$.mol$^{-1}$ is the gas constant
- $T_{0}$ (units: K) is a reference temperature
- $T$ (units: K) is the temperature
- $D(Q/K)$ is the "direction" of this reaction.  Recall that $r$ is a dissolution rate, so if $r>0$ then the kinetic mass will decrease with time.  The following choices for $D$ are available.

  - `both`: if $Q>K$ then $D=-1$ (so the kinetic species mass will increase with time), if $Q \leq K$ then $D=1$ (so the kinetic species mass will decrease with time).  This means both dissolution and precipitation are modelled, and which one occurs depends on the size of $Q$ relative to $K$.
  - `dissolution`: if $Q>K$ then $D=0$, while if $Q \leq K$ then $D=1$.  This means that precipitation is prevented, while only dissolution (with $r>0$) occurs
  - `precipitation`: if $Q>K$ then $D=-1$, while if $Q \leq K$ then $D=0$.  This means that precipitation (with $r<0$) occurs, but dissolution is prevented
  - `raw`: $D=1$ irrespective of $Q$ and $K$.  This means dissolution will occur if $k>0$, while precipitation will occur if $k<0$.
  - `death`: $D=1$ irrespective of $Q$ and $K$.  This means dissolution will occur if $k>0$, while precipitation will occur if $k<0$.  In addition, no reactants will be consumed or produced by the kinetic reaction: only the mass of the kinetic species will change.  This is used to model the death of microbes in biologically-catalysed scenarios.
  

In addition, there are auxillary inputs that do not impact the rate directly, but impact the resulting products:

- `non_kinetic_biological_catalyst` is a primary or secondary species that may be created or destroyed in addition to the reactants and products in the kinetic reaction [gen_kin_react_eqn].  This is usually a biological species, hence the name "biological catalyst".  An example is the [sulfate reducer](bio_sulfate.md) model.
- `non_kinetic_biological_efficiency` is the number of moles of the `non_kinetic_biological_catalyst` that are created per mole of [gen_kin_react_eqn] reaction turnover
- `kinetic_biological_efficiency`: when one mole of the reaction [gen_kin_react_eqn] occurs then `kinetic_biological_efficiency` moles of $A_{\bar{k}}$ is *created*.  Obviously, this defaults to $-1$, but for [biogeochemical](theory/biogeochemistry.md) models, where $A_{\bar{k}}$ represents a microbe population that is catalysing a reaction, a positive value may be appropriate.  Examples are the [sulfate reducer](bio_sulfate.md), the [arsenate reducer](bio_arsenate.md) and the [aquifer zoning](bio_zoning.md) models.


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
- do not use any `promoting_species_names` or `promoting_indices` or `promoting_monod_indices`
- ensure `kinetic_molal_index` and `kinetic_monod_index` are both zero
- set $\theta = 0$ and $\eta = 0$
- set $E_{a} = 0$ and $E_{c} = 0$
- set `direction = both`

## Example 2

Suppose that the kinetic rate for a certain mineral is proportional the mineral's surface area
\begin{equation}
r = kA
\end{equation}
and that the surface area is known and fixed.  Then set:

- $k$ to the coefficient of proportionality, with units mol.m$^{-2}$.s$^{-1}$ (assuming the simulation time is measured in seconds)
- $A$ to the known mineral surface area, with units m$^{2}$
- `multiply_by_mass = false`
- do not use any `promoting_species_names` or `promoting_indices` or `promoting_monod_indices`
- ensure `kinetic_molal_index` and `kinetic_monod_index` are both zero
- set $\theta = 0$ and $\eta = 0$
- set $E_{a} = 0$ and $E_{c} = 0$
- set `direction = both`

## Example 3

Suppose that the kinetic rate for a certain mineral is proportional the mineral's free mass
\begin{equation}
r = kM
\end{equation}
Then set

- $k$ to the coefficient of proportionality, with units mol.g$^{-1}$.s$^{-1}$ (assuming the simulation time is measured in seconds)
- $A = 1$
- `multiply_by_mass = true`.  MOOSE will automatically calculate the mineral's free mass depending on its molar mass and the number of free moles.
- do not use any `promoting_species_names` or `promoting_indices` or `promoting_monod_indices`
- ensure `kinetic_molal_index` and `kinetic_monod_index` are both zero
- set $\theta = 0$ and $\eta = 0$
- set $E_{a} = 0$ and $E_{c} = 0$
- set `direction = both`


## Example 4

Suppose that the kinetic rate for a certain mineral is proportional the mineral surface area
\begin{equation}
r = kA \ ,
\end{equation}
but that the surface area itself isn't known.  Only the mineral's specific surface area (surface area per gram of free mineral m$^{2}$.g$^{-1}$) is known.  Then set

- $k$ to the coefficient of proportionality, with units mol.m$^{-2}$.s$^{-1}$ (assuming the simulation time is measured in seconds)
- $A$ to be the mineral specific surface area, with units m$^{2}$.g$^{-1}$.
- `multiply_by_mass = true`.  MOOSE will automatically calculate the mineral's free mass depending on its molar mass and the number of free moles.
- do not use any `promoting_species_names` or `promoting_indices` or `promoting_monod_indices`
- ensure `kinetic_molal_index` and `kinetic_monod_index` are both zero
- set $\theta = 0$ and $\eta = 0$
- set $E_{a} = 0$ and $E_{c} = 0$
- set `direction = both`

## Example 5

Suppose that the kinetic rate for a certain redox couple is proportional the quantity of solvent water present:
\begin{equation}
r = kn_{w} \ ,
\end{equation}
Then set

- $k$ to the coefficient of proportionality, with units mol.kg$^{-1}$.s$^{-1}$ (assuming the simulation time is measured in seconds)
- $A = 1$ and `multiply_by_mass = false`.
- `promoting_species_names = H2O` and `promoting_indices = 1` and `promoting_monod_indices = 0`
- ensure `kinetic_molal_index` and `kinetic_monod_index` are both zero
- set $\theta = 0$ and $\eta = 0$
- set $E_{a} = 0$ and $E_{c} = 0$
- set `direction = both`


## Example 6

Suppose that the kinetic rate for a certain redox couple depends on the pH:
\begin{equation}
r = k_{\mathrm{acid}} a_{H^{+}} + k_{\mathrm{base}} a_{OH^{-}}^{1.5}  \ .
\end{equation}
For this, two `GeochemistryKineticRate` UserObjects must be created.  Each has $A=1$, `multiply_by_mass = false`, $\theta = 0$, $\eta = 0$, $E_{a}=0$, $E_{c} = 0$, `direction = both` and zero `kinetic_molal_index` and zero `kinetic_monod_index`.

- The first has $k = k_{\mathrm{acid}}$, `promoting_species_names = H+` and `promoting_species_indices = 1` and `promoting_monod_indices = 0`.
- The second has $k = k_{\mathrm{base}}$, `promoting_species_names = OH-` and `promoting_species_indices = 1.5` and `promoting_monod_indices = 0`.

These are then supplied to the [GeochemicalModelDefinition](GeochemicalModelDefinition.md) using its `kinetic_rate_descriptions` input.


## Example 7

Suppose that the kinetic rate for a certain redox couple depends on the pH, the molality of Ca$^{2+}$ via a monod expression, and the temperature:
\begin{equation}
r = k_{\mathrm{acid}} a_{H^{+}}^{1.5} \frac{m_{Ca++}^{0.3}}{(m_{Ca++}^{0.3} + K_{Ca++}^{0.3})^{2}}\exp(E_{a}/(RT)) \ .
\end{equation}
Then set

- $k = k_{\mathrm{acid}}$
- $A= 1$ and `multiply_by_mass = false`
- promoting_species_names = "H+ Ca++" and promoting_indices = "1.5 0.3" and promoting_monod_indices = "0 2"
- $K_{Ca++}$ appropriately
- $\theta = 0$ and $\eta = 0$
- $E_{a}$ appropriately
- `one_over_T0 = 0`
- $E_{c} = 0$
- `direction = both`

## Example 8

Suppose that a microbe is subject to mortality only, and no reaction products are produced:
\begin{equation}
r = k \times \mathrm{mass} \mathrm{\ \ \ (with\ no\ reaction\ products)} \ .
\end{equation}
Then set

- `intrinsic_rate_constant = k`
- `multiply_by_mass = true`
- $\eta = 0$
- `direction = death`

An example where this is used is the [aquifer zoning](bio_zoning.md) model.

## Example 9

Suppose that a microbe catalyses a reaction at rate
\begin{equation}
\label{eqn.microbe,monod}
r = kM \frac{m_{A}}{(m_{A} + K_{A})^{2}} \left(1 - \left(\frac{Q \exp(45000/RT)}{K} \right)^{1/5} \right) \ .
\end{equation}
Here $A$ is a basis or secondary equilibrium species.
Then set

- `intrinsic_rate_constant = k`
- `multiply_by_mass = true`
- `promoting_species_names = A`, `promoting_indices = 1`, `promoting_monod_indices = 2`, `promoting_half_saturation = KA`
- `theta = 0.2`
- `eta = 1`
- `energy_captured = 45000`

If, in addition, the 2 moles of microbe are created per 1 mole of reaction turnover, then

- `kinetic_biological_efficiency = 2`

This is similar to the [sulfate reducer](bio_sulfate.md) model.


!syntax parameters /UserObjects/GeochemistryKineticRate

!syntax inputs /UserObjects/GeochemistryKineticRate

!syntax children /UserObjects/GeochemistryKineticRate
