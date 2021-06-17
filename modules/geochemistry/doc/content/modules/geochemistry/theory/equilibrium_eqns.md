# Equations relating molality and mole numbers of the basis

During the mathematical solve procedure, the `geochemistry` module forms equations to relate the molality and mole number of the basis species to each other and the molality of the secondary species.

Notation and definitions are described in [geochemistry_nomenclature.md].

Let $n_{w}$ be the mass \[kg\] of solvent water.  The secondary species also contain water: one mole of secondary species $j$ contains $\nu_{wj}$ moles of water "inside it".  The same holds for sorbed species.  Therefore, so the total number of moles of water is
\begin{equation}
M_{w} = n_{w}\left(55.51 + \sum_{j}\nu_{wj}m_{j} + \sum_{q}\nu_{wq}m_{q}\right) \ .
\end{equation}
Here $55.51$ is the number of moles of H$_{2}$O in a kg of water.

Similarly, in terms of the molality $m_{i}$, the mole-number (total number of moles of substance) of [basis](basis.md) species $A_{i}$ is
\begin{equation}
M_{i} = n_{w}\left(m_{i} + \sum_{j}\nu_{ij}m_{j} + \sum_{q}\nu_{iq}m_{q}\right) \ .
\end{equation}
There are $N_{i}$ of these equations.

Let $n_{k}$ be the mole number of precipitate of mineral $k$.  (Note that $n_{w}$ is a mass, while $n_{k}$ has dimensions of moles, following the notation of [!cite](bethke_2007).)  This may be unknown, since an experiment might commence by adding a certain amount of mineral, but the reactions might produce or consume some of it.  Nevertheless, the total number of moles of [basis](basis.md) mineral $k$ is
\begin{equation}
M_{k} = n_{k} + n_{w}\left(\sum_{j}\nu_{kj}m_{j} + \sum_{q}\nu_{kq}m_{q} \right)\ .
\end{equation}
There are $N_{k}$ of these equations.

It is assumed that the gas is buffered to an infinite reservoir, so for equilibrium reactions we only ned to consider the gas components that exist in the aqueous solution as part of the secondary species:
\begin{equation}
M_{m} = n_{w} \left(\sum_{j}\nu_{mj}m_{j} + \sum_{q}\nu_{mq}m_{q}\right)\ .
\end{equation}
There are $N_{m}$ of these equations.

The total mole number of sorbing sites, $M_{p}$, may be written in terms of the molality of the sorbing sites, $m_{p}$ and the molality of the sorbed species, $m_{q}$:
\begin{equation}
M_{p} = n_{w} \left(m_{p} + \sum_{q}\nu_{pq}m_{q} \right)
\end{equation}

Without sources and kinetic contributions, the full set of equations are
\begin{equation}
\begin{aligned}
M_{w} & = n_{w}\left(55.51 + \sum_{j}\nu_{wj}m_{j} + \sum_{q}\nu_{wq}m_{q}\right) \\
M_{i} & = n_{w}\left(m_{i} + \sum_{j}\nu_{ij}m_{j} + \sum_{q}\nu_{iq}m_{q}\right) \\
M_{k} & = n_{k} + n_{w}\left(\sum_{j}\nu_{kj}m_{j} + \sum_{q}\nu_{kq}m_{q} \right) \\
M_{m} & = n_{w} \left(\sum_{j}\nu_{mj}m_{j} + \sum_{q}\nu_{mq}m_{q}\right) \\
M_{p} & = n_{w} \left(m_{p} + \sum_{q}\nu_{pq}m_{q} \right) \\
m_{j} & = \frac{a_{w}^{\nu_{wj}}\cdot \prod_{i}(\gamma_{i}m_{i})^{\nu_{ij}} \cdot \prod_{k}a_{k}^{\nu_{kj}} \cdot \prod_{m}f_{m}^{\nu_{mj}}}{\gamma_{j}K_{j}} \\
m_{q} & = \frac{1}{K_{q}\mathcal{C}} \left(a_{w}^{\nu_{wq}}\cdot \prod_{i}(\gamma_{i}m_{i})^{\nu_{iq}} \cdot \prod_{k}a_{k}^{\nu_{kq}} \cdot \prod_{m}f_{m}^{\nu_{mq}} \cdot m_{p}^{\nu_{pq}} \right) \\
\end{aligned}
\end{equation}
In addition, if surface complexation is present, there are equations for the surface potentials, $\Psi_{p}$.  In the above equations:

- $M$ \[units: mol\] is the total number of moles of a substance
- $m$ \[units: mol.kg$^{-1}$\] is the molality
- $n_{w}$ \[units: kg\] is the mass of solvent water
- $n_{k}$ \[units: mol\] is the mole number of mineral $k$
- $\nu$ \[dimensionless\] is a stoichiometric coefficient
- $a$ is an activity
- $\gamma$ is an activity coefficient
- $f$ is a gas fugacity
- $K$ is an equilibrium constant
- $\mathcal{C}$ is unity for the Langmuir approach to sorption, involves electonic charge for ion exchange, and involves the surface potential for the surface complexation approach
- Note that the last two equations are not dimensionally consistent.  Instead, it is conventional to omit dimension-full factors of "1", and simply use consistent units of moles, kg, bars and Kelvin throughout all calculations.

These equations relate $1 + N_{i} + N_{j} + N_{k} + N_{m} + N_{p} + N_{q}$ quantities, as well as $N_{p}$ additional equations for $\Psi_{p}$ if relevant.
