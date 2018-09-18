# HHPFCRFF

!syntax description /Kernels/HHPFCRFF

Implements

\begin{equation}
\pm P u,
\end{equation}

where the sign is determined by the `positive` parameter, $P$ (`prop_name`) is a
material property, and $u$ is either a coupled variable (`coupled_var`)
or - if not explicitly specified - the non-linear variable the kernel is operating on.

!syntax parameters /Kernels/HHPFCRFF

!syntax inputs /Kernels/HHPFCRFF

!syntax children /Kernels/HHPFCRFF
