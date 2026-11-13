## Volume Junction

A general conservation law equation can be expressed as

!equation
\pd{a}{t} + \nabla\cdot\mathbf{f} = 0 \eqc

where $a$ is the conserved quantity. Integrating this equation over a volume
$\Omega$ gives

!equation
\int\limits_{\Omega} \pd{a}{t} d\Omega + \int\limits_{\Omega} \nabla\cdot\mathbf{f} d\Omega = 0 \eqp

The time derivative can be expressed as follows:

!equation
\int\limits_{\Omega} \pd{a}{t} d\Omega = \ddt{\bar{a} V} \eqc

where $\bar{a}$ is the average of $a$ over $\Omega$:

!equation
\bar{a} \equiv \frac{1}{V}\int\limits_{\Omega} a d\Omega \eqc

and $V \equiv \int\limits_{\Omega} d\Omega$.

The flux term can be expressed as follows:

!equation
\int\limits_{\Omega} \nabla\cdot\mathbf{f} d\Omega
  = \int\limits_\Gamma \mathbf{f}\cdot\mathbf{n} d\Gamma \eqc

where $\Gamma$ is the surface of $\Omega$, and $\mathbf{n}$ is the outward
normal vector at a position on the surface.

Now consider that $\Gamma$ is partitioned into a number of regions:
$\Gamma = \Gamma_{\text{wall}} \cup \Gamma_{\text{flow}}$, where
$\Gamma_{\text{flow}} = \bigcup\limits_{i=1}^{N} \Gamma_{\text{flow},i}$.
The surface $\Gamma_{\text{wall}}$ corresponds to a wall, and the surface
$\Gamma_{\text{flow},i}$ corresponds to a flow surface, across which a fluid
may pass. The number of flow surfaces on the volume is denoted by $N$. See
Figure [fig:volume_junction] for an illustration.

!media thermal_hydraulics/tikz_diagrams/volume_junction.png
       id=fig:volume_junction
       caption=Volume junction illustration.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;text-align:center;

Now the surface integral can be expressed as follows:

!equation
\int\limits_\Gamma \mathbf{f}\cdot\mathbf{n} d\Gamma =
    \int\limits_{\Gamma_{\text{wall}}} \mathbf{f}\cdot\mathbf{n} d\Gamma
    + \sum\limits_{i=1}^N \int\limits_{\Gamma_{\text{flow},i}} \mathbf{f}\cdot\mathbf{n} d\Gamma \eqp

Now consider that the flux $\mathbf{f}$ is constant over each flow surface, and that
each flow surface is flat (thus making $\mathbf{n}$ constant over the surface).
Then,

!equation
\int\limits_{\Gamma_{\text{flow},i}} \mathbf{f}\cdot\mathbf{n} d\Gamma
    = \mathbf{f}_i \cdot \mathbf{n}_{J,i} A_i \eqc

where $\mathbf{n}_{J,i} = -\mathbf{n}_i$, and $\mathbf{n}_i$ is the outward-facing
(from the channel perspective) normal of channel $i$ at the interface with the
junction.

Now the wall surface integral term needs to be discussed. At this point, we
consider the particular conservation laws of interest. Starting with
conservation of mass, where $a = \rho$ and
$\mathbf{f} = \mathbf{f}^{\text{mass}} \equiv \rho \mathbf{u}$:

!equation
\int\limits_{\Gamma_{\text{wall}}} \mathbf{f}\cdot\mathbf{n} d\Gamma
    = \int\limits_{\Gamma_{\text{wall}}} \rho \mathbf{u}\cdot\mathbf{n} d\Gamma
    = 0 \eqc

owing to the wall boundary condition $\mathbf{u}\cdot\mathbf{n} = 0$.

For conservation of momentum in the $x$-direction, $a = \rho u$ and
$\mathbf{f} = \mathbf{f}^{\text{mom},x} \equiv \rho u \mathbf{u} + p \mathbf{e}_x$:

!equation
\int\limits_{\Gamma_{\text{wall}}} \mathbf{f}\cdot\mathbf{n} d\Gamma
    = \int\limits_{\Gamma_{\text{wall}}}
      \pr{\rho u \mathbf{u} + p \mathbf{e}_x}\cdot\mathbf{n} d\Gamma
    = \int\limits_{\Gamma_{\text{wall}}} p n_x d\Gamma \eqp

Then, assuming the pressure to be constant along the surface, equal to the
average pressure in the volume,

!equation
\int\limits_{\Gamma_{\text{wall}}} p n_x d\Gamma = \bar{p}\int\limits_{\Gamma_{\text{wall}}} n_x d\Gamma \eqp

Using the Gauss divergence theorem,

!equation
\int\limits_{\Gamma} \mathbf{n} d\Gamma = \mathbf{0} \eqc

!equation
\int\limits_{\Gamma_{\text{wall}}} \mathbf{n} d\Gamma
    + \sum\limits_{i=1}^N \mathbf{n}_{J,i} A_i = \mathbf{0} \eqc

!equation
\int\limits_{\Gamma_{\text{wall}}} \mathbf{n} d\Gamma
    = - \sum\limits_{i=1}^N \mathbf{n}_{J,i} A_i \eqp

Therefore,

!equation
\bar{p} \int\limits_{\Gamma_{\text{wall}}} n_x d\Gamma
    = \bar{p} \sum\limits_{i=1}^N n_{i,x} A_i \eqp

Conservation of momentum in the $y$ and $z$ directions proceeds similarly.

Finally, conservation of energy has $a = \rho E$
and $\mathbf{f} = \mathbf{f}^{\text{energy}} \equiv \pr{\rho E + p}\mathbf{u}$:

!equation
\int\limits_{\Gamma_{\text{wall}}} \mathbf{f}\cdot\mathbf{n} d\Gamma
    = \int\limits_{\Gamma_{\text{wall}}}
      \pr{\rho E + p}\mathbf{u}\cdot\mathbf{n} d\Gamma
    = 0 \eqc

again owing to wall boundary conditions.

Putting everything together and denoting the volume-average quantities with
the subscript $J$ gives

!equation
\ddt{\pr{\rho V}_J} =
    -\sum\limits_{i=1}^N \mathbf{f}^{\text{mass}}_i \cdot \mathbf{n}_{J,i} A_i \eqc

!equation
\ddt{\pr{\rho u V}_J} =
    -\sum\limits_{i=1}^N \mathbf{f}^{\text{mom},x}_i \cdot \mathbf{n}_{J,i} A_i
      - p_J \sum\limits_{i=1}^N n_{i,x} A_i \eqc

!equation
\ddt{\pr{\rho v V}_J} =
    -\sum\limits_{i=1}^N \mathbf{f}^{\text{mom},y}_i \cdot \mathbf{n}_{J,i} A_i
      - p_J \sum\limits_{i=1}^N n_{i,y} A_i \eqc

!equation
\ddt{\pr{\rho w V}_J} =
    -\sum\limits_{i=1}^N \mathbf{f}^{\text{mom},z}_i \cdot \mathbf{n}_{J,i} A_i
      - p_J \sum\limits_{i=1}^N n_{i,z} A_i \eqc

!equation
\ddt{\pr{\rho E V}_J} =
    -\sum\limits_{i=1}^N \mathbf{f}^{\text{energy}}_i \cdot \mathbf{n}_{J,i} A_i \eqp

The fluxes are computed using a numerical flux function,

!equation
\mathcal{F}(\mathbf{U}_\text{L}, \mathbf{U}_\text{R}, \mathbf{n}_{\text{L},\text{R}}, \mathbf{t}_1, \mathbf{t}_2)
  = \left[\begin{array}{c}
  f^\text{mass}_n \\
  f^{\text{mom},n}_n \\
  f^{\text{mom},t_1}_n \\
  f^{\text{mom},t_2}_n \\
  f^\text{energy}_n
  \end{array}\right] \eqc

where the vector $\mathbf{n}_{\text{L},\text{R}}$ is the outward unit vector from
the "L" state to the "R" state, and $\mathbf{t}_1$ and $\mathbf{t}_2$ are arbitrary
unit vectors forming an orthonormal basis with $\mathbf{n}_{\text{L},\text{R}}$.

The "L" state is taken to be the junction, and the "R" state is taken
to be the channel $i$:

!equation
\mathbf{U}_{\text{J},i} \equiv \left[\begin{array}{c}
  \rho_\text{J} \\
  \rho_\text{J} \mathbf{u}_\text{J} \\
  \rho_\text{J} E_\text{J}
\end{array}\right] \eqc

!equation
\mathbf{U}_i \equiv \left[\begin{array}{c}
  \rho_i \\
  \rho_i u_{i,d} \mathbf{d}_i \\
  \rho_i E_i
\end{array}\right] \eqc

!equation
\mathbf{n}_{\text{L},\text{R}} \equiv \mathbf{n}_{\text{J},i} \eqc

!equation
\left[\begin{array}{c}
  f^\text{mass}_{n,i} \\
  f^{\text{mom},n}_{n,i} \\
  f^{\text{mom},t_1}_{n,i} \\
  f^{\text{mom},t_2}_{n,i} \\
  f^\text{energy}_{n,i}
  \end{array}\right]
  = \mathcal{F}(\mathbf{U}_{\text{J},i}, \mathbf{U}_i, \mathbf{n}_{\text{J},i}, \mathbf{t}_{1,i}, \mathbf{t}_{2,i}) \eqc

where $\mathbf{d}_i$ is the local orientation vector for flow channel $i$, equal or opposite to $\mathbf{n}_i$,
and $u_{i,d}$ is the component of the channel $i$ velocity in the direction $\mathbf{d}_i$.

The input velocities here are in the global Cartesian basis $\{\mathbf{e}_x, \mathbf{e}_y, \mathbf{e}_z\}$,
whereas the numerical flux function returns momentum components in the
normal basis $\{\mathbf{n}_{\text{L},\text{R}}, \mathbf{t}_1, \mathbf{t}_2\}$,
so one must perform a change of basis afterward:

!equation
f^{\text{mom},x}_{n,i} = (f^{\text{mom},n}_{n,i} \mathbf{n}_{\text{J},i}
  + f^{\text{mom},t_1}_{n,i} \mathbf{t}_{1,i} + f^{\text{mom},t_2}_{n,i} \mathbf{t}_{2,i})
  \cdot \mathbf{e}_x \eqp

!equation
f^{\text{mom},y}_{n,i} = (f^{\text{mom},n}_{n,i} \mathbf{n}_{\text{J},i}
  + f^{\text{mom},t_1}_{n,i} \mathbf{t}_{1,i} + f^{\text{mom},t_2}_{n,i} \mathbf{t}_{2,i})
  \cdot \mathbf{e}_y \eqp

!equation
f^{\text{mom},z}_{n,i} = (f^{\text{mom},n}_{n,i} \mathbf{n}_{\text{J},i}
  + f^{\text{mom},t_1}_{n,i} \mathbf{t}_{1,i} + f^{\text{mom},t_2}_{n,i} \mathbf{t}_{2,i})
  \cdot \mathbf{e}_z \eqp

However, as shown by [!cite](hong2011) and noted in [!cite](daude2018), the choice of
junction state given above leads to spurious pressure jumps at the junction, so the
normal component of the velocity in the junction state $\mathbf{U}_{\text{J},i}$
is modified as follows:

!equation
\tilde{\mathbf{u}}_{\text{J},i} = \tilde{u}_{\text{J},i,n} \mathbf{n}_{\text{J},i}
+ u_{\text{J},i,t_1} \mathbf{t}_{1,i}
+ u_{\text{J},i,t_2} \mathbf{t}_{2,i} \eqc

!equation
\tilde{u}_{\text{J},i,n} = u_{i,n} - \mathcal{G}_i \left(u_{i,n} - u_{\text{J},i,n}\right) \eqc

with $u_{i,n} \equiv u_{i,d} \mathbf{d}_i \cdot \mathbf{n}_{\text{J},i}$ and
$u_{\text{J},i,n} \equiv \mathbf{u}_\text{J} \cdot \mathbf{n}_{\text{J},i}$ and

!equation
\mathcal{G}_i = \frac{1}{2} \left(1 - \text{sgn}(u_{i,n})\right)
\min\left(\frac{\left|u_{i,n} - u_{\text{J},i,n}\right|}{c_{\text{J},i}}, 1\right) \eqc

with $c_{\text{J},i} \equiv \max(c_i, c_\text{J})$, where $c$ is sound speed.

### Hydrostatic correction id=hydrostatic_correction

The numerical flux takes the two states $\mathbf{U}_{\text{J},i}$ and $\mathbf{U}_i$ as if
both were at the face $i$. By default they are not: the channel state is the cell average of the
element next to the junction, which is the solution at the cell center $\mathbf{x}_{c,i}$, half
a cell away from the face $\mathbf{x}_i$, and the junction state is the solution at the junction
center $\mathbf{x}_\text{J}$. For a fluid at rest under gravity, the pressure changes with the
elevation, $\nabla p = \rho \mathbf{g}$, so the pressure at the cell center differs from the
pressure at the face by $\rho_i \mathbf{g} \cdot (\mathbf{x}_{c,i} - \mathbf{x}_i)$, that is,
$\rho g \Delta z / 2$ for a vertical element of height $\Delta z$. The numerical flux sees this
difference as a pressure jump between the two sides and splits it, so the pressure at the face is
off by $\rho g \Delta z / 4$, and the fluid moves until the momentum flux of the spurious
velocity balances this error. The error is proportional to the cell height, so it is small in
absolute terms, but it has the same order as the buoyancy that drives natural circulation at
low velocities, and no choice of mesh removes it.

With the parameter `use_hydrostatic_correction`, the two states are moved to the face before the
numerical flux is computed, and the wall pressure term is changed in the same way.

The channel state at the face is the slope reconstruction of the element next to the junction,
with the scheme of the flow channel. The slopes of the primitive variables $(p, u, T)$ are
computed from the two neighbors of the element and limited. This element has only one neighbor in
the channel, so the junction is used as the other one: it is placed at the mirror point of the
cell center across the face,

!equation
\mathbf{x}'_{\text{J},i} = 2 \mathbf{x}_i - \mathbf{x}_{c,i} \eqc

and its solution is moved to that point along the hydrostatic balance, with the density and the
velocity of the junction kept:

!equation
p'_{\text{J},i} = p_\text{J} + \rho_\text{J} \, \mathbf{g} \cdot (\mathbf{x}'_{\text{J},i} - \mathbf{x}_\text{J}) \eqc
\qquad
e'_{\text{J},i} = e(p'_{\text{J},i}, \rho_\text{J}) \eqc
\qquad
T'_{\text{J},i} = T(\rho_\text{J}, e'_{\text{J},i}) \eqp

For a fluid at rest, the two one-sided slopes of the pressure are then the same, and the limiter
keeps them. The reconstructed primitive state at the face is converted to the conservative state
$\mathbf{U}^\text{f}_i$ with the flow area of the channel at the face.

The junction state at the face $i$ is the junction solution moved to the face in the same way:

!equation
p_{\text{J},i} = p_\text{J} + \rho_\text{J} \, \mathbf{g} \cdot (\mathbf{x}_i - \mathbf{x}_\text{J}) \eqc
\qquad
\rho_{\text{J},i} = \rho_\text{J} \eqc
\qquad
\mathbf{u}_{\text{J},i} = \mathbf{u}_\text{J} \eqc

!equation
e_{\text{J},i} = e(p_{\text{J},i}, \rho_\text{J}) \eqc
\qquad
E_{\text{J},i} = e_{\text{J},i} + \frac{1}{2} |\mathbf{u}_\text{J}|^2 \eqc

where $e(p, \rho)$ is given by the equation of state. This is the same construction as the ghost
cell of the pressure boundary condition. The numerical flux is then evaluated with the two face
states, $\mathcal{F}(\mathbf{U}^\text{f}_{\text{J},i}, \mathbf{U}^\text{f}_i, \mathbf{n}_{\text{J},i}, \mathbf{t}_{1,i}, \mathbf{t}_{2,i})$,
and the modification of the normal velocity described above is applied in the same way.

The wall pressure term changes because the pressure on the wall is no longer the average pressure
$p_\text{J}$ but the hydrostatic pressure
$p(\mathbf{x}) = p_\text{J} + \rho_\text{J} \, \mathbf{g} \cdot (\mathbf{x} - \mathbf{x}_\text{J})$.
By the Gauss divergence theorem,

!equation
\int\limits_{\Gamma} p \mathbf{n} \, d\Gamma = \int\limits_{\Omega} \nabla p \, d\Omega = \rho_\text{J} \mathbf{g} V \eqc

so

!equation
\int\limits_{\Gamma_{\text{wall}}} p \mathbf{n} \, d\Gamma
    = \rho_\text{J} \mathbf{g} V - \sum\limits_{i=1}^N p_{\text{J},i} \mathbf{n}_{\text{J},i} A_i \eqp

The first term is the opposite of the gravity force on the junction volume, so the two cancel
exactly, and the momentum equations with the correction are

!equation
\ddt{\pr{\rho u V}_J} =
    -\sum\limits_{i=1}^N \mathbf{f}^{\text{mom},x}_i \cdot \mathbf{n}_{J,i} A_i
      - \sum\limits_{i=1}^N p_{\text{J},i} n_{i,x} A_i \eqc

!equation
\ddt{\pr{\rho v V}_J} =
    -\sum\limits_{i=1}^N \mathbf{f}^{\text{mom},y}_i \cdot \mathbf{n}_{J,i} A_i
      - \sum\limits_{i=1}^N p_{\text{J},i} n_{i,y} A_i \eqc

!equation
\ddt{\pr{\rho w V}_J} =
    -\sum\limits_{i=1}^N \mathbf{f}^{\text{mom},z}_i \cdot \mathbf{n}_{J,i} A_i
      - \sum\limits_{i=1}^N p_{\text{J},i} n_{i,z} A_i \eqc

with the face pressures $p_{\text{J},i}$ in place of $p_\text{J}$ and no gravity source term.
The mass and energy equations do not change. With this correction, a fluid at rest with a linear
pressure profile in each element stays at rest, for any position of the faces relative to the
junction center. If the flow channels use no slope reconstruction, the channel state at the face
is the cell average, as at the flow channel boundaries, and only the junction side is corrected.
