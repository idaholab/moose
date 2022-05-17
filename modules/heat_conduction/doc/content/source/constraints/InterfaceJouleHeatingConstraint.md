# InterfaceJouleHeatingConstraint

!syntax description /Constraints/InterfaceJouleHeatingConstraint

## Description

The `InterfaceJouleHeatingConstraint` class is intended to calculate add the heat source due to Joule Heating which results from the electric potential drop across an interface.
The heat source is then added to the temperature field variable  in a coupled electro-thermal simulation.
This class is intended to be used in conjunction with [ModularGapConductanceConstraint](ModularGapConductanceConstraint.md) and [GapFluxModelPressureDependentConduction](GapFluxModelPressureDependentConduction.md), which enforce the closed gap interface requirement by checking for a postive normal pressure.
As such, the `InterfaceJouleHeatingConstraint` takes as a required argument the name of the Lagrange Multiplier variable used in the electrical contact.

!alert note title=Closed Gap Interface Assumed by this Class
The `InterfaceJouleHeatingConstraint` class should only be employed when the user is certain that electric potential flux is nonzero only when the interface gap is closed.

The heat source is calculated as a function of the electric potential flux across the interface, as given by the associated Lagrange multiplier $\lambda_{\phi}$,
\begin{equation}
  \label{eq:interfaceJH_fromLMvariable}
   q_{electric} = \sigma_e \left( \nabla \phi \right)^2 = \frac{(\lambda_{\phi})^2}{\sigma_c}
\end{equation}
where $\sigma_e$ is the harmonic mean of the electrical conductivity of the primary and secondary blocks,
\begin{equation}
  \label{eq:electricCond_harmonicMean}
  \sigma_e = \frac{2 \sigma_{primary} \sigma_{secondary}}{\sigma_{primary} + \sigma_{secondary}}
\end{equation}
following [!citep](cincotti2007modeling).
The Lagrange multiplier, passed from a separate mortar contact calculation, holds the electric potential flux
\begin{equation}
  \lambda_{\phi} = \sigma_e \nabla \phi
\end{equation}
where the harmonic mean of the electrical conductivity is the same as given in [eq:electricCond_harmonicMean].


!alert note title=Employ Consistency in Primary and Secondary Designations
Consistency in the selection of the primary boundary and secondary boundary among the electrical, thermal, and interface Joule Heating mortar contact input file compoments is recommended.

With the total interface Joule heating source determined, the fraction of the heat source applied to each block at the interface is determined as
\begin{equation}
  \label{eq:primary_interfaceJH}
  q_{primary} = -q_{electric} w_f
\end{equation}
and
\begin{equation}
  \label{eq:secondary_interfaceJH}
  q_{secondary} = -q_{electric} (1 - w_f)
\end{equation}
where w$_f$ is the user-defined weighting factor that governs how the heat source is divided between the two sides of the interface.
The use of the negative sign in [eq:primary_interfaceJH] and [eq:secondary_interfaceJH] indicates that the heat source is transferred into each block instead of away from the block.

### Steady-State Analytical Verification

Under steady state analysis assumptions, the temperature at the interface in the primary boundary material block is given by Fourier's Law
\begin{equation}
  T_{interface} = \frac{l}{k_{primary}} q_{primary} + T_{edge}
\end{equation}
where l is the length of the block, k$_{primary}$ is the thermal conductivity, and T$_{edge}$ is the prescribed temeprature boundary condition at the edge of the material block.

Similarly, the interface temperature in the secondary block material is given as
\begin{equation}
  T_{interface} = \frac{l}{k_{secondary}} q_{secondary} + T_{edge}
\end{equation}

In cases where the heat source weighting factor, [eq:primary_interfaceJH] and [eq:secondary_interfaceJH], is set to 0.5, the temperature at the interface in each block will depend on the thermal conductivity value in each block.

## Example Input File Syntax

!listing modules/heat_conduction/test/tests/interface_heating_mortar/constraint_joule_heating_single_material.i block=Constraints/interface_heating

`InterfaceJouleHeatingConstraint` should be used in conjunction with the modular gap conductance constraint, shown here,

!listing modules/heat_conduction/test/tests/interface_heating_mortar/constraint_joule_heating_single_material.i block=Constraints/electrical_contact

and the pressure-dependent gap flux conduction user object, as shown below:

!listing modules/heat_conduction/test/tests/interface_heating_mortar/constraint_joule_heating_single_material.i block=UserObjects/closed_electric

!syntax parameters /Constraints/InterfaceJouleHeatingConstraint

!syntax inputs /Constraints/InterfaceJouleHeatingConstraint

!syntax children /Constraints/InterfaceJouleHeatingConstraint

!bibtex bibliography
