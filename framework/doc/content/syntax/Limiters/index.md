# Limiters

Limiters, generally speaking, limit the slope when doing high-order (e.g. accuracy order greater than
1, e.g. non-constant polynomial) interpolations from finite volume cell
centroids to faces. This limiting is done to avoid creating oscillations in the
solution field in regions of steep gradients or discontinuities. Slope limiters,
or flux limiters, are generally employed to make the solution Total Variation
Diminishing (TVD). Borrowing notation from
[here](https://en.wikipedia.org/wiki/Total_variation_diminishing), the Total
Variation when space and time have been discretized can be defined as

\begin{equation}
TV(u^n) = TV(u(\centerdot,t^n)) = \sum_j \vert u_{j+1}^n - u_j^n \vert
\end{equation}

where $u$ is the discretized approximate solution, $n$ denotes the time index,
and $u_j^n = u(x_j,t^n)$. A numerical method is TVD if

\begin{equation}
TV(u^{n+1}) \leq TV(u^n)
\end{equation}


The following limiters are available in MOOSE. We have noted the convergence
orders of each (when considering that the solution is smooth), whether they are
TVD, and what the functional form of the flux limiting function $\beta(r_f)$ is.

!table id=limiter_summary caption=Limiter Overview
|Limiter class name | Convergence Order | TVD | $\beta(r)$ |
|------------------ | ----------------- | --- | --- |
| `VanLeer`          | 2                 | Yes | $\frac{r_f +\text{abs}(r_f)}{1 + \text{abs}(r_f)}$ |
| `Upwind`           | 1                 | Yes | 0 |
| `CentralDifference` | 2                | No | 1 |
| `MinMod`           | 2                 | Yes | $\text{max}(0, \text{min}(1, r_f))$ |
| `SOU`              | 2                 | No | $r_f$ |
| `QUICK`            | 2                 | No | $\frac{3+r_f}{4}$ |
