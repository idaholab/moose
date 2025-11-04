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

Different formulations are used for compressible and incompressible/weakly-compressible
flow.

## Limiting Process for Compressible Flow

Borrowing notation from [!citep](greenshields2010implementation), we will now
discuss computation of limited quantities, represented by $\bm{\Psi}_{f\pm}$ where
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
[limiter_summary_compressible]. $r_{\pm}$ is computed as follows

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

!table id=limiter_summary_compressible caption=Compressible Limiter Overview
| Limiter class name  | Convergence Order | TVD | $\beta(r)$                                   |
| ------------------- | ----------------- | --- | -------------------------------------------- |
| `VanLeer`           | 2                 | Yes | $\frac{r +\text{abs}(r)}{1 + \text{abs}(r)}$ |
| `Upwind`            | 1                 | Yes | 0                                            |
| `CentralDifference` | 2                 | No  | 1                                            |
| `MinMod`            | 2                 | Yes | $\text{max}(0, \text{min}(1, r))$            |
| `SOU`               | 2                 | No  | $r$                                          |
| `QUICK`             | 2                 | No  | $\frac{3+r}{4}$                              |

## Limiting Process for Incompressible and Weakly-Compressible flow

A full second-order upwind reconstruction is used for incompressible and weakly-compressible solvers. In this reconstruction, the limited quantity at the face is expressed as follows:

\begin{equation}
\bm{\Psi}_f = \bm{\Psi}_C + \beta(r) ((\nabla \bm{\Psi})_C \cdot \bm{d}_{fC})
\end{equation}

where:

- $\bm{\Psi}_f$ is the value of the variable at the face
- $\bm{\Psi}_C$ is the value of the variable at the cell
- $(\nabla \bm{\Psi})_C$ is the value of the gradient at the cell, which is computed with second-order methods (Green-Gauss without skewness correction and Least-Squares for skewness corrected)
- $\bm{d}_{fC}$ is the distance vector from the face to the cell used in the interpolation
- $\beta(r)$ is the limiting function

Two kinds of limiters are supported: slope-limited and face-value limited. These limiters are defined below.

For slope-limiting, the approximate gradient ratio (or flux limiting ratio) $r$ is defined as follows:

\begin{equation}
r = 2 \frac{\bm{d}_{NC} \cdot (\nabla \bm{\Psi})_C}{\bm{d}_{NC} \cdot (\nabla \bm{\Psi})_f} - 1
\end{equation}

where:

- $\bm{d}_{NC}$ is the vector between the neighbor and current cell adjacent to the face
- $(\nabla \bm{\Psi})_f$ is the gradient of the variable at the face, which is computed by linear interpolation of second-order gradients at the adjacent cells to the face

For face-value limiting, the limiting function is defined as follows:

\begin{equation}
r =
\begin{cases}
    \frac{|\Delta_f|}{\Delta_{\text{max}}} & \text{if } \Delta_f > 0 \\
    \frac{|\Delta_f|}{\Delta_{\text{min}}} & \text{if } \Delta_f \leq 0
\end{cases}
\end{equation}

where:

- $\Delta_f = (\nabla \bm{\Psi})_C \cdot \bm{d}_{fC}$ is the increment at the face
- $\Delta_{\text{max}} = \bm{\Psi}_{\text{max}} - \bm{\Psi}_C$ is the maximum increment
- $\Delta_{\text{min}} = \bm{\Psi}_{\text{min}} - \bm{\Psi}_C$ is the minimum increment

The maximum and minimum variable values, $\Delta_{\text{max}}$ and $\Delta_{\text{min}}$, respectively, are computed with a two-cell stencil. In this method, the maximum value is determined as the maximum cell value of the two faces adjacent to the face and their neighbors, respectively. Similarly, the minimum value is computed as the minimum cell value for these cells.

Each of the limiters implemented along with the implementation reference, limiting type, whether they are TVD, and the functional form of the flux limiting function $\beta(r)$ is shown in [limiter_summary_incompressible].

!table id=limiter_summary_incompressible caption=Incompressible/Weakly-Compressible Limiter Overview
| Limiter class name                              | Limiting Type | TVD | $\beta(r)$                                                        |
| ----------------------------------------------- | ------------- | --- | ----------------------------------------------------------------- |
| `VanLeer` [!citep](harten1997)                  | Slope         | Yes | $\frac{r +\text{abs}(r)}{1 + \text{abs}(r)}$                      |
| `MinMod` [!citep](harten1997)                   | Slope         | Yes | $\text{max}(0, \text{min}(1, r))$                                 |
| `QUICK` [!citep](harten1997)                    | Slope         | Yes | $\text{min}(1,\text{max}(\text{min}(\frac{1 + 3r}{4}, 2r, 2),0))$ |
| `SOU` [!citep](harten1997)                      | Face-Value    | No  | $\text{min}(1,1/r)$                                               |
| `Venkatakrishnan` [!citep](venkatakrishnan1993) | Face-Value    | No  | $\frac{2r+1}{r(2r+1)+1}$                                          |

To illustrate the performance of the limiters, a dispersion analysis is developedand presented in [dispersion].
This consists of the advection of a passive scalar in a Cartesian mesh at 45 degrees.
The exact solution, without numerical diffusion, is a straight line at 45 degrees
dividing the regions with a scalar concentration of 1 and 0.

!alert note
In general, we recomment using `VanLeer` and `MinMod` limiters for most of the
applications considering that they provide truly bounded solutions.

!media framework/finite_volume/dispersion.png
      style=display: block;margin-left:auto;margin-right:auto;width:40%;
      id=dispersion
      caption=Dispersion problem, advection in a Cartesian mesh at 45 degrees.

The results and performance of each of the limiters are shown in [dispersion_line].
This image provides an idea of the limiting action and results that
can be expected for each of the limiters.

!media framework/finite_volume/dispersion_line.png
      style=display: block;margin-left:auto;margin-right:auto;width:40%;
      id=dispersion_line
      caption=Performance of each of the limiters in a line perpendicular to the advection front.

!bibtex bibliography
