# Side Reaction Postprocessor

!syntax description /Postprocessors/NormalBoundaryDisplacement

## Description

`NormalBoundaryDisplacement` computes average or maximum normal displacement on a boundary.
It permits modes: `average absolute_average max absolute_max` which are as follows:

`average`:
\begin{equation}
  I = \frac{1}{S \kappa}\int_{\partial S} \vec{n}^T ~ \vec{d}  dS / \kappa,
\end{equation}
where $S$ is the surface area, $\vec{n}$ is the normal vector, $\vec{d}$ is the displacement vector, and $\kappa$ is a normalization factor.

`absolute_average`:
\begin{equation}
  I = \frac{1}{S \kappa} \int_{\partial S} \left | \vec{n}^T ~ \vec{d} \right|  dS / \kappa.
\end{equation}

`max`:
\begin{equation}
  I = \frac{1}{\kappa}\max\limits_{S}  \vec{n}^T ~ \vec{d}  .
\end{equation}

`absolute_max`:
\begin{equation}
  I = \frac{1}{\kappa} \max\limits_{S}  \left | \vec{n}^T ~ \vec{d} \right|  .
\end{equation}

The normalization factor can be set by the parameter `normalization`. If true then $\kappa = \sqrt{S}$, otherwise $\kappa = 1$.

!syntax parameters /Postprocessors/NormalBoundaryDisplacement

!syntax inputs /Postprocessors/NormalBoundaryDisplacement

!syntax children /Postprocessors/NormalBoundaryDisplacement
