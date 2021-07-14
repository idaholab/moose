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

## Limiting Process

Borrowing notation from [!citep](greenshields2010implementation), we will now
discuss computation of limited quanties, represented by $\bm{\Psi}_{f\pm}$ where
$+$ represents one side of a face, and $-$ represents the other side. To be
clear about notation: the equations that follow will have a lot of $\pm$ and
$\mp$. When computing the top quantity (e.g. $+$ for $\pm$) we select the top
quantities throughout the equation, e.g. we select $+$ for $\pm$ and $-$ for
$\mp$. Similarly, when computing bottom quantities we select the bottom
quantities throughout the equation. We will also have a series of "ors" in the
text. In general left of "or" will be for top quantities and right of "or" will
be for bottom quantities.

Interpolation of limited quantities proceeds as follows:

\begin{equation}
\bm{\Psi}_{f\pm} = \left(1 - g_{f\pm}\right)\bm{\Psi}_{\pm} +
g_{f\pm}\bm{\Psi}_{\mp}
\end{equation}

where $\bm{\Psi}_{\pm}$ denotes the $+$ or $-$ cell centroid value of the
interpolated quantity and

\begin{equation}
g_{f\pm} = \beta\left(r_{\pm}\right)\left(1 - w_{f\pm}\right)
\end{equation}

where $\beta\left(r_{\pm}\right)$ represents a flux limiter function and

\begin{equation}
\label{eq:weighting}
w_{f\pm} = \vert \bm{d}_{f\mp}\vert/\left(\vert \bm{d}_{f+}\vert +
\vert\bm{d}_{f-}\vert\right)
\end{equation}

where $\vert\bm{d}_{f-}\vert$ is the norm of the distance from the face to the
$-$ cell centroid and $\vert\bm{d}_{f+}\vert$ is the norm of the distance from
the face to the $+$ cell centroid. Note that this definition of $w_{f\pm}$
differs slightly from that given in [!citep](greenshields2010implementation) in
which the denominator is given by $\vert\bm{d}_{-+}\vert$, the norm of the
distance between the $-$ and $+$ cell centroids. The definition given in
[eq:weighting] guarantees that $w_{f+} + w_{f-} = 1$. Note that for a
non-skewed mesh the definition in [eq:weighting] and
[!citep](greenshields2010implementation) are the same.

The flux limiter function $\beta(r_{\pm})$ takes different forms as shown in
[limiter_summary]. $r_{\pm}$ is computed as follows

\begin{equation}
r_{\pm} = 2 \frac{\bm{d}_{\pm}\cdot\left(\nabla
\bm{\Psi}\right)_{\pm}}{\left(\nabla_d \bm{\Psi}\right)_{f\pm}} - 1
\end{equation}

where $\left(\nabla \bm{\Psi}\right)_{\pm}$ corresponds to the $+$ or $-$ cell
centroid gradient and $\left(\nabla_d \bm{\Psi}\right)_{f\pm} =
\bm{\Psi}_{\mp} - \bm{\Psi}_{\pm}$.

The following limiters are available in MOOSE. We have noted the convergence
orders of each (when considering that the solution is smooth), whether they are
TVD, and what the functional form of the flux limiting function $\beta(r)$ is.

!table id=limiter_summary caption=Limiter Overview
|Limiter class name | Convergence Order | TVD | $\beta(r)$ |
|------------------ | ----------------- | --- | --- |
| `VanLeer`          | 2                 | Yes | $\frac{r +\text{abs}(r)}{1 + \text{abs}(r)}$ |
| `Upwind`           | 1                 | Yes | 0 |
| `CentralDifference` | 2                | No | 1 |
| `MinMod`           | 2                 | Yes | $\text{max}(0, \text{min}(1, r))$ |
| `SOU`              | 2                 | No | $r$ |
| `QUICK`            | 2                 | No | $\frac{3+r}{4}$ |

!bibtex bibliography
