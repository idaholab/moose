# FunctionNodalAverageIC

!syntax description /ICs/FunctionNodalAverageIC

This IC is typically used when a variable exists both as an elemental variable
and a nodal variable, where the following relation is desired to hold:
\begin{equation}
 y^{elem}_0 = \frac{1}{N_{node}}\sum\limits_i^{N_{node}} y_0(x_i) ,
\end{equation}
where $y_0(x)$ is the initial condition function,
$y^{elem}_0$ is the computed elemental value,
$x_i$ is the location of node $i$, and
$N_{node}$ is the number of nodes for the element.

!syntax parameters /ICs/FunctionNodalAverageIC

!syntax inputs /ICs/FunctionNodalAverageIC

!syntax children /ICs/FunctionNodalAverageIC

!bibtex bibliography
