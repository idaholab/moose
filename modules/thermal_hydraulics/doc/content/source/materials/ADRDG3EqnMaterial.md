# ADRDG3EqnMaterial

!syntax description /Materials/ADRDG3EqnMaterial

This material is responsible for computing the reconstructed solution values.
First, slopes are computed for the primitive variables $\mathbf{W}$,
limited with a slope limiter, if specified, and then used to compute the extrapolated values
at the cell interfaces:
\begin{equation}
  \mathbf{W}_{i+1/2} = \mathbf{W}_i + (x_{i+1/2} - x_i)\Delta\mathbf{W}_i \,.
\end{equation}
Then, the solution values at the interfaces are computed from these values:
\begin{equation}
  \mathbf{U}_{i+1/2} = \mathbf{U}(\mathbf{W}_{i+1/2}) \,.
\end{equation}

!syntax parameters /Materials/ADRDG3EqnMaterial

!syntax inputs /Materials/ADRDG3EqnMaterial

!syntax children /Materials/ADRDG3EqnMaterial
