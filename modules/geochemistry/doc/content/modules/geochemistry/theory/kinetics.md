# Kinetics of mineralisation, redox reactions and complex species

Notation and definitions are described in [geochemistry_nomenclature.md].

The material below follows Chapters 16 and 17 of [!cite](bethke_2007).

In addition to the [equilibrium](equilibrium.md) system, there may be components whose dynamics are governed by kinetic rate laws.  Denote these by $A_{\bar{k}}$.  For instance, $A_{\bar{k}}$ could involve minerals that slowly precipitate or dissolve, [decoupled redox reactions](basis.md) that occur slowly, or reactions that involve slow surface complexations.

## Mathematical equations to solve

Denote the kinetic rates of reactions of these components by $r_{\bar{k}}$ \[mol.s$^{-1}$\], so
\begin{equation}
\frac{\mathrm{d}n_{\bar{k}}}{\mathrm{d}t} = -r_{\bar{k}} \ .
\end{equation}
Here $n_{\bar{k}}$ is the mole number of the component $A_{\bar{k}}$.

Denote the stoichiometric coefficients by $\nu_{\ast \bar{k}}$, so that the equations determining the bulk composition are:
\begin{equation}
\begin{aligned}
\frac{\mathrm{d}M_{w}}{\mathrm{d}t} &= \sum_{\bar{k}}\nu_{w\bar{k}}r_{\bar{k}} \\
\frac{\mathrm{d}M_{i}}{\mathrm{d}t} &= \sum_{\bar{k}}\nu_{i\bar{k}}r_{\bar{k}} \\
\frac{\mathrm{d}M_{k}}{\mathrm{d}t} &= \sum_{\bar{k}}\nu_{k\bar{k}}r_{\bar{k}} \\
\frac{\mathrm{d}M_{m}}{\mathrm{d}t} &= \sum_{\bar{k}}\nu_{m\bar{k}}r_{\bar{k}} \\
\frac{\mathrm{d}M_{p}}{\mathrm{d}t} &= \sum_{\bar{k}}\nu_{p\bar{k}}r_{\bar{k}}
\end{aligned}
\end{equation}
I shall give examples of $r_{\bar{k}}$ and $\nu_{\ast\bar{k}}$ below, since they are different for different specific cases.  I suspect the equation for $M_{m}$ is ignored, since once the [Newton-Raphson procedure](equilibrium.md) has provided the secondary and sorbed species' molality, $M_{m}$ is defined uniquely.  Any excess or deficit according to the ODE comes from the infinite gas buffer of fixed fugacity.

As described in the chapter on [equilibrium](equilibrium.md), the equations involving $M_{k}$ and $M_{m}$ are simple to solve, and only the equations involving $M_{w}$, $M_{i}$ and $M_{p}$ need to considered, and for the equilibrium case, these are
\begin{equation}
\begin{aligned}
M_{w} & = n_{w}\left(55.51 + \sum_{j}\nu_{wj}m_{j} + \sum_{q}\nu_{wq}m_{q}\right) \\
M_{i} & = n_{w}\left(m_{i} + \sum_{j}\nu_{ij}m_{j} + \sum_{q}\nu_{iq}m_{q}\right) \\
M_{p} & = n_{w} \left(m_{p} + \sum_{q}\nu_{pq}m_{q} \right)
\end{aligned}
\end{equation}
These must be solved for $(n_{w}, m_{i}, m_{p})$ given $(M_{w}, M_{i}, M_{p})$.  In the kinetic case, these equations are replaced by
\begin{equation}
\begin{aligned}
0 & = n_{w}(t)\left(55.51 + \sum_{j}\nu_{wj}m_{j}(t) + \sum_{q}\nu_{wq}m_{q}(t)\right) - M_{w}(t-\Delta t) - \Delta t \sum_{\bar{k}}\nu_{w\bar{k}}r_{\bar{k}}(t) \\
0 & = n_{w}(t)\left(m_{i}(t) + \sum_{j}\nu_{ij}m_{j}(t) + \sum_{q}\nu_{iq}m_{q}(t)\right) - M_{i}(t-\Delta t) - \Delta t \sum_{\bar{k}}\nu_{i\bar{k}}r_{\bar{k}}(t) \\
0 & = n_{w}(t)\left(m_{p}(t) + \sum_{q}\nu_{pq}m_{q}(t)\right) - M_{p}(t-\Delta t) - \Delta t \sum_{\bar{k}}\nu_{p\bar{k}}r_{\bar{k}}(t) \\
0 & = n_{\bar{k}}(t) - n_{\bar{k}}(t - \Delta t) + \Delta t\, r_{\bar{k}}(t) \ .
\end{aligned}
\end{equation}
The implicit time-stepping method has been used here, since the last term on each line involves $r(t)$.  In Chapter 16 of [!cite](bethke_2007), he advocates using a "$\theta=1/2$" method where this term is $\frac{1}{2}(r(t) + r(t-\Delta t))$ but I don't see any advantage of that.  The under-relaxation of the [equilibrium solution process](equilibrium.md) should presumably be enhanced to ensure positivity of all quantities.

## Rates and stoichiometry for mineralisation

Consider one or more minerals $A_{\bar{k}}$ whose rates of dissolution and precipitation are controlled by kinetic laws.  They are a subset of the $A_{l}$.  They are not in equilibrium with the system, so do not appear in the basis.  Their kinetically-controlled reaction may be written
\begin{equation}
A_{\bar{k}} \rightleftharpoons \nu_{w\bar{k}}A_{w} + \sum_{i}\nu_{i\bar{k}}A_{i} + \sum_{k}\nu_{k\bar{k}}A_{k} + \sum_{m}\nu_{m\bar{k}}A_{m} \ .
\end{equation}
This reaction is written in the [database](database.md), just like for the other $A_{l}$.  It defines the stoichiometric coefficients $\nu_{\ast\bar{k}}$ used above.

### Approach 1

[!cite](bethke_2007) writes the rate of dissolution as
\begin{equation}
r_{\bar{k}} = -\frac{\mathrm{d}n_{\bar{k}}}{\mathrm{d}t} = S_{\bar{k}}k_{\bar{k}} \prod_{\bar{j}} (m_{\bar{j}})^{P_{\bar{j}\bar{k}}} \left[ 1 - \left(\frac{Q_{\bar{k}}}{K_{\bar{k}}}\right) \right] \ .
\end{equation}
Here:

- $r_{\bar{k}}$ \[mol.s$^{-1}$\] is the reaction rate
- $n_{\bar{k}}$ \[mol\] is the mole number of the mineral $A_{\bar{k}}$
- $t$ \[s\] is time
- $S_{\bar{k}}$ \[m$^{2}$\] is the mineral's surface area.  Usually this is very difficult to determine, and may be incorrectly estimate by orders of magnitude.  It is difficult to translate lab results into natural environments.  Usually, a specific surface area (m$^{2}$.kg$^{-1}$) is specified, and this is multipled by the mass of the mineral to yield $A_{\bar{k}}$.  Hence $A_{\bar{k}}$ is time-dependent and alters during the NR iteration.  Alternatively, $A_{\bar{k}}$ could be simply constant.
- $k_{\bar{k}}$ \[mol.s$^{-1}$.m$^{-2}$\] is the reaction's intrinsic rate constant
- The species $A_{\bar{j}}$, which may be basis species, secondary species, etc, are the rate law's promoting or inhibiting species.  Here $m_{\bar{j}}$ is understood to stand for molality (with the corresponding dimensional inconsistency in the above equation), but when $A_{\bar{j}}$ is H$^{+}$ or OH$^{-}$ it is activity, and when $A_{\bar{j}}$ is a gas it represents fugacity.
- $P_{\bar{j}\bar{k}}$ \[dimensionless\] are empirically-determined exponents
- $Q_{\bar{k}}$ is the reaction's activity product
- $K_{\bar{k}}$ is the reaction's equilbrium constant

Precipitation is assumed to follow the same rate as dissolution.


### Approach 2

[!cite](palandri) write (dropping the $\bar{k}$ subscripts)
\begin{equation}
r = S \sum_{b}a_{b}^{n_{b}}k_{0,b} \exp\left( \frac{E_{b}}{R}\left( \frac{1}{T_{0}} - \frac{1}{T} \right) \right) \left| 1 - \left(\frac{Q}{K}\right)^{\theta_{b}} \right|^{\eta_{b}} \ .
\end{equation}
There are a lot of terms in this expression:
- $S_{\bar{k}}$ \[m$^{2}$\] is the mineral's surface area.
- $b$ indicates the $b^{\mathrm{th}}$ aqueous component.
- $a$ \[dimensionless\] is activity
- $n$ \[dimensionless\] is an index, which [!cite](palandri) write for many reactions of interest
- $k_{0,b}$ \[mol.m$^{-2}$.s$^{-1}$\] is the Arrhenius pre-factor, which [!cite](palandri) write for many reactions of interest
- $E_{b}$ \[J.mol$^{-1}$\] is the activation energy for component $b$, which [!cite](palandri) write for many reactions of interest
- $R = 8.314\ldots\,$J.K$^{-1}$.mol$^{-1}$ is the gas constant
- $T$ \[K\] is temperature and $T_{0}$ is a reference temperature
- $K$ \[dimensionless\] is the reaction's equilibrium constant, quantified through the [database](database.md)
- $Q$ \[dimensionless\] is the reaction's activity product
- $\theta_{b}$ and $\eta_{b}$ are two dimensionless indices, which [!cite](palandri) write for many reactions of interest.

In reality, the sum-over-$b$ is actually restricted to, at most, a sum over H$^{+}$, H$_{2}$O and OH$^{-1}$.  Actually, instead of using OH$^{-}$, [!cite](palandri) use H$^{+}$ again, and write
\begin{equation}
\begin{aligned}
r/A = & a_{\mathrm{H}^{+}}^{n_{\mathrm{acid}}} k_{\mathrm{acid}}\exp \left(\frac{E_{\mathrm{acid}}}{R}\left(\frac{1}{T_{0}} - \frac{1}{T}\right) \right) \left| 1 - \left(\frac{Q}{K}\right)^{\theta_{\mathrm{acid}}} \right|^{\eta_{\mathrm{acid}}} \\
& +  k_{\mathrm{ntrl}}\exp \left(\frac{E_{\mathrm{ntrl}}}{R}\left(\frac{1}{T_{0}} - \frac{1}{T}\right) \right) \left| 1 - \left(\frac{Q}{K}\right)^{\theta_{\mathrm{ntrl}}} \right|^{\eta_{\mathrm{ntrl}}} \\
& + a_{\mathrm{H}^{+}}^{n_{\mathrm{base}}} k_{\mathrm{base}}\exp \left(\frac{E_{\mathrm{base}}}{R}\left(\frac{1}{T_{0}} - \frac{1}{T}\right) \right) \left| 1 - \left(\frac{Q}{K}\right)^{\theta_{\mathrm{base}}} \right|^{\eta_{\mathrm{base}}}
\end{aligned}
\end{equation}
Once again, precipitation is assumed to follow the same rate as dissolution.

### General approach

Clearly, there could easily be other reaction rate formulae that are important in GeoTES settings.  Therefore, the MOOSE implementation will probably use a user-defined reaction rate.

## Redox kinetics

When redox pairs are in disequilibrium (and hence decoupled) the redox reaction may be modelled using a kinetic approach.  The most general form for a redox reaction includes the solvent water, basis aqueous species, secondary aqueous species, minerals, gases, etc, so can be written as
\begin{equation}
0 \rightarrow \bar{\nu}_{w\bar{k}}A_{w} + \sum_{i}\bar{\nu}_{i\bar{k}}A_{i}
 + \sum_{j}\bar{\nu}_{j\bar{k}}A_{j}
 + \sum_{l}\bar{\nu}_{l\bar{k}}A_{l}
 + \sum_{n}\bar{\nu}_{n\bar{k}}A_{n}
 + \sum_{p}\bar{\nu}_{p\bar{k}}A_{p}
 + \sum_{q}\bar{\nu}_{q\bar{k}}A_{q}
\end{equation}
Here, I've used a bar on the stoichiometric coefficients, to distinguish them from the $\nu_{\ast\bar{k}}$ used above.  For example, $\bar{\nu}_{w\bar{k}} \neq \nu_{w\bar{k}}$ because some water can also exist in secondary species, minerals, etc that get created/destroyed by these kinetically-controlled redox reactions.  This notation is the opposite of Chapter 17 of [!cite](bethke_2007).

Since there are $\nu_{wj}$ moles of water in each mole of secondary species $j$, $\nu_{wl}$ moles of water in each mole of mineral $l$, $\nu_{wn}$ moles of water in gas $n$, etc, then
\begin{equation}
\nu_{w\bar{k}} = \bar{\nu}_{w\bar{k}}
+ \sum_{j}\bar{\nu}_{j\bar{k}}\nu_{wj}
+ \sum_{l}\bar{\nu}_{l\bar{k}}\nu_{wl}
+ \sum_{n}\bar{\nu}_{n\bar{k}}\nu_{wn}
+ \sum_{p}\bar{\nu}_{p\bar{k}}\nu_{wp}
+ \sum_{q}\bar{\nu}_{q\bar{k}}\nu_{wq}
\end{equation}
(We know that $\nu_{wi}=0$ so it is not included in the above.)  Similarly, there are $\nu_{ij}$ moles of basis aqueous species $i$ in 1 mole of secondary species $j$, etc, so
\begin{equation}
\begin{aligned}
\nu_{i\bar{k}} & = \bar{\nu}_{i\bar{k}}
+ \sum_{j}\bar{\nu}_{j\bar{k}}\nu_{ij}
+ \sum_{l}\bar{\nu}_{l\bar{k}}\nu_{il}
+ \sum_{n}\bar{\nu}_{n\bar{k}}\nu_{in}
+ \sum_{p}\bar{\nu}_{p\bar{k}}\nu_{ip}
+ \sum_{q}\bar{\nu}_{q\bar{k}}\nu_{iq} \\
\nu_{k\bar{k}} & = 0
+ \sum_{j}\bar{\nu}_{j\bar{k}}\nu_{kj}
+ \sum_{l}\bar{\nu}_{l\bar{k}}\nu_{kl}
+ \sum_{n}\bar{\nu}_{n\bar{k}}\nu_{kn}
+ \sum_{p}\bar{\nu}_{p\bar{k}}\nu_{kp}
+ \sum_{q}\bar{\nu}_{q\bar{k}}\nu_{kq}
\end{aligned}
\end{equation}
The first term is zero in the last equation because the $\sum_{l}$ contains $k$ already.

### Approach 1

The reaction's activity product is
\begin{equation}
Q_{\bar{k}} = a_{w}^{\bar{\nu}_{w\bar{k}}}
\cdot \prod_{i}a_{i}^{\bar{\nu}_{i\bar{k}}}
\cdot \prod_{j}a_{j}^{\bar{\nu}_{j\bar{k}}}
\cdot \prod_{l}a_{l}^{\bar{\nu}_{l\bar{k}}}
\cdot \prod_{n}f_{n}^{\bar{\nu}_{n\bar{k}}}
\cdot \prod_{p}m_{p}^{\bar{\nu}_{p\bar{k}}}
\cdot \prod_{q}m_{q}^{\bar{\nu}_{q\bar{k}}}
\end{equation}
and the reaction rate is
\begin{equation}
r_{\bar{k}} = n_{w}k_{+,\bar{k}}\prod_{\bar{j}}(m_{\bar{j}})^{P_{\bar{j}\bar{k}}} \left[ 1 - \left(\frac{Q_{\bar{k}}}{K_{\bar{k}}}\right)^{\omega} \right]
\end{equation}
Here:

- $r_{\bar{k}}$ \[mol.s$^{-1}$\] is the reaction rate
- $n_{w}$ \[kg\] is the mass of solvent water
- $k_{+,\bar{k}}$ \[mol.s$^{-1}$.kg$^{-1}$\] is reaction rate constant
- The species $A_{\bar{j}}$, which may be basis species, secondary species, etc, are the rate law's promoting or inhibiting species.  Here $m_{\bar{j}}$ is understood to stand for molality (with the corresponding dimensional inconsistency in the above equation), but when $A_{\bar{j}}$ is H$^{+}$ or OH$^{-}$ it is activity, and when $A_{\bar{j}}$ is a gas it represents fugacity.
- $P_{\bar{j}\bar{k}}$ \[dimensionless\] are empirically-determined exponents
- $Q_{\bar{k}}$ is the reaction's activity product
- $K_{\bar{k}}$ is the reaction's equilbrium constant
- $\omega$ \[dimensionless\] is an empirically-determined exponent

### Approach 2

[!cite](bethke_2007) also writes a verion of redox kinetics involving enzymes, in which the reaction rate involves a Michaelis-Menten term.

### General approach

The MOOSE approach will employ a user-defined function that can evaluate the rate.


!bibtex bibliography
