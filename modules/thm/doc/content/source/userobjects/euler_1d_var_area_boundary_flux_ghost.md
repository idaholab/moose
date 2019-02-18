This boundary flux $\mathbf{F}_b$ is computed using a numerical flux function $\mathcal{F}$ with the interior solution $\mathbf{U}_i$ and the ghost cell
solution $\mathbf{U}_b$:
\begin{equation}
  \mathbf{F}_b = \mathcal{F}(\mathbf{U}_i, \mathbf{U}_b, \mathbf{n}) \,,
\end{equation}
where $\mathbf{n}$ is the outward-facing normal vector.
