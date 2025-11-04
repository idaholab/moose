# Coupling electromagnetics and heat transfer modules for wire heating

The purpose of this example is to present how the [electromagnetics](modules/electromagnetics/index.md)
and [heat transfer](modules/heat_transfer/index.md) modules can be coupled together to simulate the heating of
materials through electromagnetic effects. In particular, the example presented here involves simulating the
heating profile of a copper wire when suppled with the fusing current.

## Problem Description

The problem of interest involves suppling large amounts of DC current to a copper wire, such that the temperature
of the wire reaches the melting point. To model this be behavior, the follow equations are utilized:

\begin{equation}
  \nabla \times \nabla \times \vec{A} + j \mu \omega \left( \sigma \vec{A} \right) = \vec{J}_{source}
\end{equation}
\begin{equation}
  \rho C_{p} \frac{\partial T}{\partial t} - \nabla \cdot k \nabla T = Q
\end{equation}

Where:

- $\vec{A}$ is the magnetic vector potential,
- $j$ is $\sqrt{-1}$,
- $\mu$ is the permeability of free space,
- $\omega$ is the angular frequency of the system (60 Hz for this example),
- $\sigma$ is the electric conductivity of the wire,
- $\vec{J}_{source}$ is the supplied DC current density,
- $\rho$ is the density of copper,
- $C_{p}$ is the heat capacity of copper,
- $T$ is the temperature,
- $k$ is the thermal conductivity of the wire, and
- $Q$ is the Joule heating source.

The Joule heating, $Q$, is defined by the magnitude of the electric field, $E$, such that:
\begin{equation}
  Q = \frac{1}{2} \sigma \lvert \vec{E} \rvert ^{2}
\end{equation}

Instead of solving for the electric field directly, this example used the magnetic vector potential
because the magnetic vector potential formulation allows for the directly implementation of a
suppled DC current density. This implementation of a suppled DC current density can be better displayed by looking at
the relationship between the electric field and the magnetic vector potential:

\begin{equation}
  \vec{E} = -j \omega \vec{A} - \nabla V
\end{equation}

The third term, $\text{-}\nabla V$ is the gradient of the electrostatic potential. This
electrostatic potential is the source of the DC current, which is defined as:

\begin{equation}
  \vec{J}_{source} = - \sigma \nabla V
\end{equation}

Since the magnitude of the electric field is the term of interest for Joule heating, [AuxKernels](AuxKernels/index.md) are
used to calculate the electric field magnitude from the magnetic vector potential and the supplied
DC current density.

\begin{equation}
  \lvert \vec{E} \rvert = \lvert -j \omega \vec{A} \rvert + \lvert \frac{\vec{J}_{source}}{\sigma} \rvert
\end{equation}

!alert! note
We cannot define the supplied current density as a volumetric term when solving for the electric field directly, as this would result in an ill-posed electric field.
!alert-end!

## Boundary Conditions

The boundary conditions for the magnetic vector potential and temperature are:

\begin{equation}
  \vec{n} \times \nabla \times \vec{A} = 0
\end{equation}
\begin{equation}
  \vec{q} \cdot \vec{n} = h \left( T - T_{\infty} \right)
\end{equation}

Where:

- $\vec{n}$ is the normal unit vector at the boundary surface,
- $\vec{q}$ is the heat flux,
- $h$ is the convective heat transfer coefficient (10 W/m$^{2}$K for this example), and
- $T_{\infty}$ is the far-field temperature (293 K for this example).

## Example Input File

!listing modules/combined/test/tests/electromagnetic_joule_heating/fusing_current_through_copper_wire.i
