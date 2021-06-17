# Equilibrium equations and solution method

Notation and definitions are described in [geochemistry_nomenclature.md].

This page follows [!cite](bethke_2007).

## Notation

Introduce the following:

- $A_{w}$: water.
- $A_{i}$: aqueous [basis species](basis.md).  This is a label, not a quantity.  for example, one of the $A_{i}$ might be Na$^{+}$.
- $A_{j}$: other aqueous species, which are called the "secondary" species.
- $A_{k}$: minerals in the equilibrium system of reactions that appear in the [basis](basis.md).
- $A_{l}$: all minerals, even those that do not exist in the equilibrium system.  This might include minerals whose reactions are kinetically controlled.
- $A_{m}$: gases of known fugacity that appear in the [basis](basis.md).
- $A_{n}$: all gases.
- $A_{q}$: sorbed species.  These do not get transported --- they are surface complexes.
- $A_{p}$: sorbing sites.  In the Langmuir approch and ion-exchange approach there is just one of these, and all sorbing species compete to sorb.  In the "surface complexation" approach, there are an equal number of $A_{p}$ as $A_{q}$.

## Basis

Note that the index $i$, $j$, etc, gives meaning to the symbol that it is indexing.  Although unusual in mathematics literature, this is the common convention in geochemistry.  There are $1+N_{i}+N_{k}+N_{m}+N_{p}$ unknowns and the basis is
\begin{equation}
\mathrm{basis} = (A_{w}, A_{i}, A_{k}, A_{m}, A_{p}) \ .
\end{equation}
Being a basis, all other other components can be expressed in terms of it.

## Secondary species

All secondary species are in equilibrium with the basis.  They are not sorbing, so
\begin{equation}
A_{j} \rightleftharpoons \nu_{wj}A_{w} + \sum_{i}\nu_{ij}A_{i} + \sum_{k}\nu_{kj}A_{k} + \sum_{m}\nu_{mj}A_{m} \ .
\end{equation}
This equation is specified in the [database](geochemistry/database/index.md) with the $\nu$ being the stoichiometric reaction coefficients.  The database specifies the equilibrium constant, $K_{j}$, and [mass-action equilibrium](equilibrium_reactions.md) reads
\begin{equation}
K_{j} = \frac{a_{w}^{\nu_{wj}}\cdot \prod_{i}(\gamma_{i}m_{i})^{\nu_{ij}} \cdot \prod_{k}a_{k}^{\nu_{kj}} \cdot \prod_{m}f_{m}^{\nu_{mj}}}{\gamma_{j}m_{j}} \ .
\end{equation}
Here:

- $a_{w}$ is the [activity of water](activity_coefficients.md)
- $\gamma_{i}m_{i}$ is the [activity](activity_coefficients.md) of the primary species $i$
- $a_{k}$ is the activity of mineral $k$, which is [assumed to be unity](activity_coefficients.md) but is written here for clarity
- $f_{m}$ is the known gas [fugacity](fugacity.md)
- $m_{j}$ is the molality of the secondary species
- $\gamma_{j}$ is the [activity coefficient](activity_coefficients.md) of the secondary species.
- $m$ denotes [molality](geochemistry_nomenclature.md)

As discussed on the [activity](activity_coefficients.md) and [fugacity](fugacity.md) pages, geochemists typically ignore the dimensional inconsistencies in this equation.  Instead, it is conventional to omit dimension-full factors of "1", and simply use consistent units of moles, kg, bars and Kelvin throughout all calculations.

The above equation may be rearranged for the molality of the secondary species
\begin{equation}
m_{j} = \frac{a_{w}^{\nu_{wj}}\cdot \prod_{i}(\gamma_{i}m_{i})^{\nu_{ij}} \cdot \prod_{k}a_{k}^{\nu_{kj}} \cdot \prod_{m}f_{m}^{\nu_{mj}}}{\gamma_{j}K_{j}} \ .
\end{equation}

## Minerals, activity product and saturation index

For any mineral (including those in the basis, but the equation is trivial for them)
\begin{equation}
A_{l} \rightleftharpoons \nu_{wl}A_{w} + \sum_{i}\nu_{il}A_{i} + \sum_{k}\nu_{kl}A_{k} + \sum_{m}\nu_{ml}A_{m} \ .
\end{equation}
Define the activity product
\begin{equation}
Q_{l} = \frac{a_{w}^{\nu_{wl}}\cdot \prod_{i}(\gamma_{i}m_{i})^{\nu_{il}} \cdot \prod_{k}a_{k}^{\nu_{kl}} \cdot \prod_{m}f_{m}^{\nu_{ml}}}{a_{l}} \ ,
\end{equation}
which may be compared with the expression for the reaction's equilibrium constant, $K_{l}$.  Define the "saturation index":
\begin{equation}
\mathrm{SI}_{l} = \log_{10}\left(Q_{l}/K_{l}\right) \ .
\end{equation}
There are three possibilities:

- $\mathrm{SI}=0$.  The mineral is at equilibrium.
- $\mathrm{SI}>0$.  The mineral is supersaturated, which means the system will try to precipitate (perhaps slowly) the mineral.
- $\mathrm{SI}<0$.  The mineral is undersaturated, which means the aqueous solution will (perhaps slowly) dissolve more of the mineral.


## Sorption

### $K_{d}$ and Feundlich approaches

Here, the basis does not include any $A_{p}$.  The equations are summarised in Chapter 9 of [!cite](bethke_2007).  However, Bethke recommends against using these approaches, since adsorption may occur without limit.

### Langmuir approach

Here a single sorption site is introduced into the basis ($N_{p}=1$) and the reaction for each sorbed species, $A_{q}$, written in terms of the basis is
\begin{equation}
A_{q} \rightleftharpoons \nu_{wq}A_{w} + \sum_{i}\nu_{iq}A_{i} + \sum_{k}\nu_{kq}A_{k} + \sum_{m}\nu_{mq}A_{m} + \nu_{pq}A_{p} \ .
\end{equation}
Here $\nu_{pq}$ is generally unity, but is written here for consistency of notation.  Each such reaction has an associated equilibrium constant, $K_{q}$, and a mass action that reads
\begin{equation}
m_{q} = \frac{1}{K_{q}} \left(a_{w}^{\nu_{wq}}\cdot \prod_{i}(\gamma_{i}m_{i})^{\nu_{iq}} \cdot \prod_{k}a_{k}^{\nu_{kq}} \cdot \prod_{m}f_{m}^{\nu_{mq}} \cdot m_{p}^{\nu_{pq}} \right) \ .
\end{equation}
Here $m_{p}$ is the molality of unoccupied sites and $m_{q}$ is the molality of the secondary species.

### Ion exchange

Ion exchange does not treat the sorption of a species on the surface of the porous skeleton, but the replacement of one adsorped ion with another.  The development of the theory is similar to the Langmuir approach of sorption however.  A new basis species representing exchange sites, $A_{p}$ is introduced.  The formula for $m_{q}$ is similar to the Langmuir equation above, but involves the electric charge of the ions involved in the exchange as well as multiplicative factors.

The ion-exchange approach is not currently supported in the `geochemistry` module.

### Surface complexation approach

Here, the basis includes an entry, $A_{p}$ for *each type* of surface site considered, and there is a reaction to form each surface complex $A_{q}$:
\begin{equation}
A_{q} \rightleftharpoons \nu_{wq}A_{w} + \sum_{i}\nu_{iq}A_{i} + \sum_{k}\nu_{kq}A_{k} + \sum_{m}\nu_{mq}A_{m} + \nu_{pq}A_{p} \ .
\end{equation}
The mass action for each surface complex $A_{q}$ is
\begin{equation}
\label{eq:mq}
m_{q} = \frac{1}{K_{q}e^{z_{a}F\Psi/RT}} \left(a_{w}^{\nu_{wq}}\cdot \prod_{i}(\gamma_{i}m_{i})^{\nu_{iq}} \cdot \prod_{k}a_{k}^{\nu_{kq}} \cdot \prod_{m}f_{m}^{\nu_{mq}} \cdot m_{p}^{\nu_{pq}} \right) \ .
\end{equation}
which sets the molality of each surface complex.  Here:

- $z_{q}$ \[dimensionless\] is the electronic charge of the surface complex $A_{q}$
- $\Psi$ \[J.C$^{-1}$ = V\] is the surface potential, which is set as below
- $F = 96485\ldots \,$C.mol$^{-1}$ is the Faraday constant
- $R = 8.314\ldots\,$J.K$^{-1}$.mol$^{-1}$ is the gas constant
- $T$ \[K\] is temperature

Computing the surface potential, $\Psi$, requires an additional equation, which [!cite](bethke_2007) writes as
\begin{equation}
\label{eq:psi}
\frac{A_{\mathrm{sf}}}{Fn_{w}}\sqrt{8RT\epsilon\epsilon_{0}\rho_{w} I}\sinh \left(\frac{z_{\pm}\Psi F}{2RT}\right) = \sum_{q}z_{q}m_{q} \ .
\end{equation}
Note that the right-hand side of this equation depends on $\Psi$ through [eq:mq].  The additional notation introduced here is as follows.

- $A_{\mathrm{sf}}$ \[m$^{2}$\] is the surface area of material upon which the $A_{q}$ live.  Usually a mineral in the system provides this surface area and usually the user specifies a specific surface area in \[m$^{2}$/g(of mineral)\].  Hence $A_{\mathrm{sf}}$ is proportional to the mole number of a mineral, $n_{k}$, with coefficient of proportionality being the mineral's density \[g.mol$^{-1}$\] and the specific surface area.
- $\epsilon$ \[dimensionless\] is the dielectric constant: $\epsilon = 78.5$ at 25$^{\circ}$C for water.
- $\epsilon_{0} = 8.854\times 10^{-12}\,$F.m$^{-1}$ is the permittivity of free space.
- $\rho_{w}=1000\,$kg.m$^{-3}$ is the density of water.
- $I$ is the ionic strength.
- $z_{\pm}=1$ is the charge on the background solute.  [!cite](bethke_2007) assumes $z_{\pm}=1$
- $z_{q}$ \[dimensionless\] is the charge of the surface complex $A_{q}$.

## Equations relating molality and mole numbers of the basis

Equations may be formed to relate the molality and mole number of the basis species to each other and the molality of the secondary species.

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

Let $n_{k}$ be the mole number of mineral $k$.  (Note that $n_{w}$ is a mass, while $n_{k}$ has dimensions of moles: I follow the notation of [!cite](bethke_2007).)  This may be unknown, since an experiment might commence by adding a certain amount of mineral, but the reactions might produce or consume some of it.  Nevertheless, the total number of moles of [basis](basis.md) mineral $k$ is
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

## Full equations, assumptions, and reduced equations

In summary, the full set of equations are
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
In addition, if surface complexation is present, [eq:psi] provides an equation for the surface potential, $\Psi$.  In the above equations:

- $M$ \[mol\] is the total number of moles of a substance
- $m$ \[mol.kg$^{-1}$\] is the molality
- $n_{w}$ \[kg\] is the mass of solvent water
- $n_{k}$ \[mol\] is the mole number of mineral $k$
- $\nu$ \[dimensionless\] is a stoichiometric coefficient
- $a$ is an activity
- $\gamma$ is an activity coefficient
- $f$ is a gas fugacity
- $K$ is an equilibrium constant
- $\mathcal{C}$ is unity for the Langmuir approach to sorption, involves electonic charge for ion exchange, and involves the surface potential for the surface complexation approach
- The last two equations are not dimensionally consistent: see the note above.

These equations relate $1 + N_{i} + N_{j} + N_{k} + N_{m} + N_{p} + N_{q}$ quantities, as well as the addition equation for $\Psi$ if relevant.  Once the basis species are known, the molalities of the secondary and any sorbed species may be worked out simply using the final two equations.

The known conditions for the reactions can take a varity of forms:

- For H$_{2}$O: either $M_{w}$ (total number of moles) or $n_{w}$ (number of moles acting as the solute) is known.
- For each primary species: either $M_{i}$ (bulk-composition number of moles) or $m_{i}$ (molality of free primary species) is known.  E.g. specifying pH specifies $m_{H^{+}}$.  Charge balance must hold.
- For each mineral: either $M_{k}$ (bulk-composition number of moles) or $n_{k}$ (number of moles of basis mineral $k$) is known
- Because an infinite buffer reservoir of gas is assumed, the initial concentration of gas is irrelevant --- the user simply specifies the fixed fugacity
- For each sorption site: either $M_{p}$ or $m_{p}$ is known.

To solve these equations, the following assumptions are made.

- The mineral's activities are all unity.  This means the secondary species' molalities and sorbed species' molalities do not depend on the mineral activities.  This means that, given the secondary species' molalities and sorbed species' molalities, the equation $M_{k} = n_{k} + n_{w}\left(\sum_{j}\nu_{kj}m_{j} + \sum_{q}\nu_{kq}m_{q} \right)$ may be used to provide $n_{k}$ in terms of known $M_{k}$, or $M_{k}$ in terms of known $n_{k}$, depending on the problem setup.  Therefore, this equation is trivial and may be ignored during the solution process.  It is simply used once the solution is found.

- The gas fugacities $f_{m}$ are known and fixed.  For the same reason as the previous bullet point, this means that the equation $M_{m} = n_{w} \left(\sum_{j}\nu_{mj}m_{j} + \sum_{q}\nu_{mq}m_{q}\right)$ may be ignored during the solution process.  It is simply used to provide $M_{m}$ (the total number of moles of gas in the aqueous solution) once the solution is found.

- The [activity coefficients](activity_coefficients.md) are known functions.

- If surface complexation is employed, $A_{sf}$ is fixed (see [eq:psi]).  If $A_{\mathrm{sf}}$ depends on $n_{k}$, as is often the case, this breaks the "splitting into reduced equations" just described.  However, the only examples I have found are when $n_{k}$ is fixed by specifying the amount of free mineral.

This means that the "reduced" equations are
\begin{equation}
\begin{aligned}
M_{w} & = n_{w}\left(55.51 + \sum_{j}\nu_{wj}m_{j} + \sum_{q}\nu_{wq}m_{q}\right) \\
M_{i} & = n_{w}\left(m_{i} + \sum_{j}\nu_{ij}m_{j} + \sum_{q}\nu_{iq}m_{q}\right) \\
M_{p} & = n_{w} \left(m_{p} + \sum_{q}\nu_{pq}m_{q} \right) \ ,
\end{aligned}
\end{equation}
as well as [eq:psi] if appropriate.
The problem becomes: given $(M_{w}, M_{i}, M_{p})$, find $n_{w}$, $m_{i}$ and $m_{p}$ (and $\Psi$, if appropriate).  The Newton-Raphson (NR) procedure (below) is used.  If $n_{w}$ is known, the first equation is trivial, and needn't appear in the NR.  Similarly, if any $m_{i}$ or $m_{p}$ are known (e.g. pH is fixed by the user) then that equation needn't appear in the NR.

## Solution method

The NR procedure is used to find the $(n_{w}, m_{i}, m_{p})$ in terms of the $(M_{w}, M_{i}, M_{p})$.  [!cite](bethke_2007) describes some detail and some useful procedures that help convergence.  

### Initial configuration

The initial configuration for the $(n_{w}, m_{i}, m_{p})$ is whichever is the most appropriate of:

- set by the user;
- 90% of the specified values for $(M_{w}, M_{i}, M_{p})$;
- the result of the previous time step;
- if the residual for basis species $i$ is extremely large, then the starting $m_{i}$ are repeatedly halved until the residual reaches a manageable size of less than $10^{3}$.

### Activity

At every step in the NR procedure, activity coefficients $\gamma_{i}$ and $\gamma_{j}$ as well as the activity of water, $a_{w}$, are computed based on the current values of $m_{i}$ and $m_{j}$.  The derivatives of the activity coefficients with respect to $m$ are not included in the NR Jacobian.

### Under-relaxation

Because the quantities $(n_{w}, m_{i}, m_{p})$ can never be negative, the NR iteration is under-relaxed.  At iteration number $q$, define
\begin{equation}
\frac{1}{\delta} = \mathrm{max}\left(1, -\frac{\Delta n_{w}}{n_{w}^{(q)}/2}, -\frac{\Delta m_{i}}{m_{i}^{(q)}/2}, -\frac{\Delta m_{p}}{m_{p}^{(q)}/2} \right) \ ,
\end{equation}
then the NR update takes the form
\begin{equation}
\begin{aligned}
n_{w}^{(q+1)} & = n_{w}^{(q)} + \delta\, \Delta n_{w} \\
m_{i}^{(q+1)} & = m_{i}^{(q)} + \delta\, \Delta m_{i} \\
m_{p}^{(q+1)} & = m_{p}^{(q)} + \delta\, \Delta m_{p}
\end{aligned}
\end{equation}

### Charge balance

If a basis-species free molality is specified (such as $m_{\mathrm{H}^{+}}$) then charge balance cannot be assured until NR has converged.  To force electrical neutrality at the end of each NR iteration, the equation
\begin{equation}
\sum_{i}z_{i}M_{i} = 0 \ ,
\end{equation}
(where $z_{i}$ is the charge of basis species $A_{i}$) is used to set the bulk concentration of one of the basis species.  That is, one of the equations
\begin{equation}
M_{i} = n_{w}\left(m_{i} + \sum_{j}\nu_{ij}m_{j} + \sum_{q}\nu_{iq}m_{q}\right) \ ,
\end{equation}
is replaced by this charge-balance equation, to set the bulk concentration, $M$, of a basis species in abundant concentration for which the greatest analytic uncertainty exists.  This is generally Cl$^{-}$, but can be set by the user.  See Section 4.3 and 14.2 of [!cite](bethke_2007).

### Mineral undersaturation and supersaturation

After NR convergence, and substituting the result to find $n_{k}$ and $M_{k}$, it may be found that $n_{k}<0$, meaning the mineral was completely consumed (and more).  Similarly, computing the saturation index of minerals not in the basis may reveal a supersaturated mineral that should be allowed to precipitate.  Therefore, after NR convergence, the following procedure involving [basis swaps](swap.md) is used.

- All $n_{k}$ are computed.  If one or more a negative, then the one that is the most negative is removed from the basis.  It is replaced by secondary species $A_{j}$ that satisfies $\mathrm{max}_{j}(m_{j}|\nu_{kj}|)$.  The NR procedure is resolved.

- The sauration index of each mineral, $A_{l}$, that can form in a reaction model is checked for supersaturation.  If one or more are supersaturated, the one with the largest saturation index is added into the basis, and something removed from the basis.  This "something is": preferably, a primary species satisfying $\mathrm{max}_{i}(|\nu_{il}|/m_{i})$; if no primary species appears in the reaction for $A_{l}$ then the mineral in the reaction for $A_{l}$ that satisfies $\mathrm{min}_{k}(n_{k}/\nu_{kl})$.  The NR procedure is re-solved (and checked for undersaturation and supersaturation, etc).  Finally, due to a multitute of complexities, this procedure involving supersaturation is undesirable --- the supersaturated mineral is ignored because the modeller knows it doesn't occur in reality.

### Primary species with low concentration

Sometimes a species in the basis can occur at very small concentration, leading to numerical instability because making small corrections to its molality leads to large deviations in the molalities of the secondary species.  In this case, it may be [swapped](swap.md) for an abundant secondary species.  [!cite](bethke_2007) doesn't give conditions for when this type of swap should occur, or whether it's supposed to be an automatic or user-controlled process

### Final check

After NR convergence, and all the other checks above have been satisfied, the following checks are made:

- the masses of each component are conserved
- the activity product is equal to the equilibrium constant for each reaction







!bibtex bibliography
