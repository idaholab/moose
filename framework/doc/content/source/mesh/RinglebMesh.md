# RinglebMesh

## Overview

This mesh can be applied to a Ringleb problem. This problem tests the spatial accuracy of high-order methods. The flow is transonic and smooth. The geometry is also smooth, and high-order curved boundary representation appears to be critical.

## Governing Equations

The governing equations are the 2D Euler equations with $\gamma = 1.4$.

## Geometry

Let $k$ be a streamline parameter, i.e., $k = constant$ on each streamline. The two stream lines for the two wall boundaries are $k=k_{max}=1.5$ for the inner wall, and $k=k_{min}=0.7$ for the outer wall. Let $q$ be the velocity magnitude. For each fixed $k$ , $k_{min} \leq k \leq k_{max}$, the variable $q$  varies between $Q_0=0.5$ and $k$ . For each $q$ , define the speed of sound $a$ , density $\rho$ , pressure $p$, and a quantity denoted by $J$ by:
\begin{equation*}
a=\sqrt{1 - \frac{\gamma-1}{2}q^2};\quad \rho = a^{\frac{2}{\gamma-1}};\quad p = \frac{1}{\gamma}a^{\frac{2 \gamma}{\gamma-1}};\quad J = \frac{1}{a} + \frac{1}{3a^3} + \frac{1}{5a^5} - \frac{1}{2}\log \frac{1+a}{1-a}
\end{equation*}

For each pair $(q,k)$, set:
\begin{equation*}
x(q,k) = \frac{1}{2\rho}\left( \frac{2}{k^2}-\frac{1}{q^2} \right) - \frac{J}{2}
\quad \mathrm{and} \quad
y(q,k) = \pm \frac{1}{k \rho q} \sqrt{1-\frac{q^2}{k^2}}
\end{equation*}

## Mesh Overlook

For example, let's consider the following input file:

```
[Mesh]
  type = RinglebMesh
  kmin = 0.7
  num_k_pts = 9
  num_q_pts = 20
  kmax = 1.2
  n_extra_q_pts = 2
  gamma = 1.4
  triangles = true
[]
```

The corresponding mesh looks like this:

!media large_media/ringleb_mesh/ringleb_mesh.png
       style=width:50%;

## Further RinglebMesh Documentation

!syntax parameters /Mesh/RinglebMesh

!syntax inputs /Mesh/RinglebMesh

!syntax children /Mesh/RinglebMesh