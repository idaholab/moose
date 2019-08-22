# Sublattice KKS model

The sublattice Kim-Kim-Suzuki (SLKKS) [!cite](Schwen2021) model is an extension
of the original KKS model incorporaring an additional equal chemical potential
constrain among the sublattice concentration in each phase.

As such each component in the system will have corresponding phase
concentrations, which are split up into per-sublattice concentrations.

\begin{equation}
c_i = \sum_j h(\eta_j)\sum_k a_{jk} c_{ijk},
\end{equation}

where $i$ indexes a component, $j$ a phase, and $k$ a sublattice in the given
phase. $a_{jk}$ is the fraction of $k$ sublattice sites in phase $j$. We assume
that the sublattices in a given phase are always in equilibrium, thus

\begin{equation}
\frac{\partial F_j}{\partial c_{ijk_\alpha}} = \frac{\partial F_j}{\partial c_{ijk_\beta}} \quad ,\quad \forall \{k_\alpha, k_\beta\}.
\end{equation}

At the same time the original KKS model chemical potential equilibria still hold

\begin{equation}
\frac{\partial F}{\partial c_i} = \frac{\partial F_j}{\partial c_{ij_\alpha k_\beta}} \quad ,\quad \forall \{k_\alpha, k_\beta\}.
\end{equation}
