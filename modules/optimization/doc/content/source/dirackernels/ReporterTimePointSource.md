# ReporterTimePointSource

!syntax description /DiracKernels/ReporterTimePointSource

## Overview

A `ReporterTimePointSource` reads in multiple point sources from a [Reporter](Reporters/index.md) or [VectorPostprocessor](VectorPostprocessors/index.md).  The point source values and coordinates are updated as the values are changed.

For exposition, the data determining the coordinates and values can be represented as a five-column matrix:

!equation
\mathbf{S} \equiv
\begin{bmatrix}
x_1 & y_1 & z_1 & t_1 & s_1 \\
x_2 & y_2 & z_2 & t_2 & s_2 \\
\vdots & \vdots & \vdots & \vdots & \vdots \\
x_N & y_N & z_N & t_N & s_N
\end{bmatrix} ,

where $N$ is the number of 4-dimensional coordinates supplied. $x_i$, $y_i$, $z_i$, and $t_i$ are the x, y, z, and time value at coordinate $i$, respectively. And $s_i$ is the source value at coordinate $i$. During the simulation, only the sources where $t_i$ matches the simulation time ($t_{\mathrm{sim}}$) are applied. So at particular $t_{\mathrm{sim}}$ during the simulation the sources being applied can be represented as:

!equation id=eq:coord_value
\vec{S}_i(t_{\mathrm{sim}}) = 
\begin{bmatrix}
x_i & y_i & z_i & s_i\delta_{t_i, t_{\mathrm{sim}}}
\end{bmatrix},\quad
i=1,...,N.

For maximum flexibility, the coordinates and values can change during the simulation based on the evaluation of these vectors in the [Reporter](Reporters/index.md) or [VectorPostprocessor](VectorPostprocessors/index.md). As such, $\mathbf{S}$ can depend on time ($\mathbf{S}(t)$). So [!eqref](eq:coord_value) can be representated as:

!equation
\vec{S}_i(t_{\mathrm{sim}}) = 
\begin{bmatrix}
x_i(t_{\mathrm{sim}}) & y_i(t_{\mathrm{sim}}) & z_i(t_{\mathrm{sim}}) & s_i(t_{\mathrm{sim}})\delta_{t_i(t_{\mathrm{sim}}), t_{\mathrm{sim}}}
\end{bmatrix},\quad
i=1,...,N(t_{\mathrm{sim}}).

!alert note
It is important for the `ReporterTimePointSource` to never use a [VectorPostprocessor](VectorPostprocessors/index.md) with [!param](/VectorPostprocessors/PointValueSampler/contains_complete_history)` = true`, as this can modify the ordering of the coordinates and points.

## Example Input Syntax

An example of a `ReporterTimePointSource` using a [ConstantReporter](/ConstantReporter.md):

!listing reporter_time_point_source.i block=DiracKernels Reporters

This reporter essentially creates:

!equation
\mathbf{S} = 
\begin{bmatrix}
x    & y    & z    & t    & s \\
\hline
0.25 & 0.25 & 0.25 & 0.10 & 0.00 \\
0.75 & 0.25 & 0.25 & 0.10 & 1.00 \\
0.25 & 0.75 & 0.25 & 0.10 & 2.00 \\
0.75 & 0.75 & 0.25 & 0.10 & 3.00 \\
0.25 & 0.25 & 0.75 & 0.10 & 4.00 \\
0.75 & 0.25 & 0.75 & 0.10 & 5.00 \\
0.25 & 0.75 & 0.75 & 0.10 & 6.00 \\
0.75 & 0.75 & 0.75 & 0.10 & 7.00 \\
0.25 & 0.25 & 0.25 & 0.20 & 8.00 \\
0.75 & 0.25 & 0.25 & 0.20 & 9.00 \\
0.25 & 0.75 & 0.25 & 0.20 & 10.0 \\
0.75 & 0.75 & 0.25 & 0.20 & 11.0 \\
0.25 & 0.25 & 0.75 & 0.20 & 12.0 \\
0.75 & 0.25 & 0.75 & 0.20 & 13.0 \\
0.25 & 0.75 & 0.75 & 0.20 & 14.0 \\
0.75 & 0.75 & 0.75 & 0.20 & 15.0 \\
0.25 & 0.25 & 0.25 & 0.30 & 16.0 \\
0.75 & 0.25 & 0.25 & 0.30 & 17.0 \\
0.25 & 0.75 & 0.25 & 0.30 & 18.0 \\
0.75 & 0.75 & 0.25 & 0.30 & 19.0 \\
0.25 & 0.25 & 0.75 & 0.30 & 20.0 \\
0.75 & 0.25 & 0.75 & 0.30 & 21.0 \\
0.25 & 0.75 & 0.75 & 0.30 & 22.0 \\
0.75 & 0.75 & 0.75 & 0.30 & 23.0 \\
\end{bmatrix}.

During the simulation, only the values where the time value (fourth column) is applied. So at $t_{\mathrm{sim}}=0.10$ only the first eight rows are applied.

!syntax parameters /DiracKernels/ReporterTimePointSource

!syntax inputs /DiracKernels/ReporterTimePointSource

!syntax children /DiracKernels/ReporterTimePointSource
