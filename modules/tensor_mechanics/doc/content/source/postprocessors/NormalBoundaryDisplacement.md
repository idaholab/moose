# NormalBoundaryDisplacement

!syntax description /Postprocessors/NormalBoundaryDisplacement

## Description

`NormalBoundaryDisplacement` computes average or maximum normal displacement on a boundary.
It permits modes: `average absolute_average max absolute_max` which are as follows:

`average`:
\begin{equation}
  I = \frac{1}{S}\int_{\partial S} \vec{n}^T ~ \vec{d}  dS,
\end{equation}
where $S$ is the surface area, $\vec{n}$ is the normal vector, and $\vec{d}$ is the displacement vector.

`absolute_average`:
\begin{equation}
  I = \frac{1}{S} \int_{\partial S} \left | \vec{n}^T ~ \vec{d} \right|  dS.
\end{equation}

`max`:
\begin{equation}
  I = \max\limits_{S}  \vec{n}^T ~ \vec{d}  .
\end{equation}

`absolute_max`:
\begin{equation}
  I = \max\limits_{S}  \left | \vec{n}^T ~ \vec{d} \right|  .
\end{equation}

!syntax parameters /Postprocessors/NormalBoundaryDisplacement

!syntax inputs /Postprocessors/NormalBoundaryDisplacement

!syntax children /Postprocessors/NormalBoundaryDisplacement
