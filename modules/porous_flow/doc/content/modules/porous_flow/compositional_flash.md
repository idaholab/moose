# Compositional flash calculations

The miscible flow models available in PorousFlow use a compositional flash to
determine the amount of each fluid phase present for the given set of persistent
primary variables using the Rachford-Rice equation [!citep](rachford-rice1952)

\begin{equation}
\label{eq:rr}
\sum_i \frac{z_i (K_i - 1)}{1 + V (K_i - 1)} = 0,
\end{equation}
where $z_i$ is the total mass fraction of fluid component $i$ summed over all fluid
phases
\begin{equation}
z_i = \frac{\sum_{\alpha} S_{\alpha} \rho_{\alpha} x_{\alpha}^{i}}{\sum_{\alpha} S_{\alpha} \rho_{\alpha}},
\end{equation}
$K_i$ is the equilibrium constant
\begin{equation}
K_i = \frac{y_i}{x_i}
\end{equation}
that relates the mass fraction of fluid component $i$ in the gas phase ($y_i$) to the mass
fraction in the liquid phase ($x_i$), and $V$ is the mass fraction of fluid in the gas
phase, which for a two-phase model is
\begin{equation}
V = \frac{S_g \rho_g}{S_g \rho_g + S_l \rho_l}
\end{equation}
where $S_g$ and $S_l$ are the saturations of the gas and liquid phases, respectively, and
$\rho_g$ and $\rho_l$ are the densities of the gas and liquid phases, respectively.

The Rachford-Rice equation, [eq:rr], is solved for $V$, after which the unknown
gas saturation can be calculated. The Rachford-Rice equation can be solved analytically for
the case where there are two fluid components, whereby $V$ is
\begin{equation}
V = \frac{z_i (K_1 - K_0) - (K_1 - 1)}{(K_1 - 1)(K_0 - 1)}.
\end{equation}

For cases where there are more than two fluid components, however, [eq:rr] must be solved numerically. Fortunately, the Rachford-Rice equation has the nice numerical property
that [eq:rr] is monotonically varying with $V$, so that numerical solution requires
only a few iterations, and is therefore numerically inexpensive.

Once the vapor mass fraction $V$ has been calculated, the mass fractions of fluid
component $i$ in each phase can be calculated
\begin{equation}
x_i = \frac{z_i}{(K_i - 1) V + 1}, \quad y_i = \frac{z_i K_i}{(K_i - 1) V + 1}.
\end{equation}


!bibtex bibliography
