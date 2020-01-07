# Worked example of Kuzmin-Turek stabilization

This page is part of a set of pages devoted to discussions of numerical stabilization in PorousFlow.  See:

- [Numerical stabilization lead page](stabilization.md)
- [Mass lumping](mass_lumping.md)
- [Full upwinding](upwinding.md)
- [Kuzmin-Turek stabilization](kt.md)
- [Numerical diffusion](numerical_diffusion.md)
- [A worked example of Kuzmin-Turek stabilization](kt_worked.md)

Kuzmin and Turek [!citep](KuzminTurek2004) describe a method of stabilising advection while minimising artificial numerical diffusion.  In this page "Kuzmin and Turek" is abbreviatved to "KT".  This page will make much more sense if you read it in tandem with the KT paper!  KT consider a single scalar quantity $u$ that is being advected.  For sake of argument, in this page we think of $u$ as "heat".

In this page, the 1D example studied in the [numerical diffusion page](numerical_diffusion.md) is used to explicitly illustrate how their approach works.  The input file is

!listing modules/porous_flow/test/tests/numerical_diffusion/fltvd.i

The mesh sits in the domain $0\leq x \leq 1$ and is meshed with 100 elements.  The initial condition is $u(x)=1$ if $0.1\leq x \leq 3$, and $u(x)=0$ otherwise.  The velocity is uniform to the right: $v=0.1$.

The key differences between this input file and any other that simulated advection is the use of the [FluxLimitedTVDAdvection](kernels/FluxLimitedTVDAdvection.md) Kernel and the [AdvectiveFluxCalculatorConstantVelocity](AdvectiveFluxCalculatorConstantVelocity.md) UserObject.  The latter computes $K$, $R^{+}$ and $R^{-}$ that are used by the Kernel as described in detail in this page.

The above input file sets the `flux_limiter_type = superbee`, but different types, such as `none`, `vanleer`, `minmod` or `mc` may be chosen.  As explained in detail below, these add antidiffusion to counteract the artificial numerical diffusion added to stabilize the problem (except for the `none` choice that adds no antidiffusion).

KT work with a lumped mass matrix (see discussion at the start of KT Section 5, Eqn (25)), so the input file uses the [MassLumpedTimeDerivative](MassLumpedTimeDerivative.md) Kernel.


## No stabilization

Without any stabilization, the KT method is just the usual Galerkin finite-element method used by default in MOOSE.  The advection equation
\begin{equation}
M\dot{u} + \nabla\cdot(\mathbf{v}u) = 0 \ ,
\end{equation}
is discretised (employing mass-lumping) to
\begin{equation}
\label{eq:discadv}
m_{i}\dot{u}_{i} = \sum_{j}k_{ij}u_{j} \ ,
\end{equation}
where $m_{i}$ is the lumped mass at node $i$ and $u_{i}$ is the value of $u$ at node $i$.  In this equation, KT have introduced the transport matrix, $K$, with entries
\begin{equation}
k_{ij} = -\int_{\Omega}\psi_{i}\nabla(\mathbf{v}_{j} \psi_{j}) \ .
\end{equation}
Here $\psi_{i}$ is the test function at node $i$ and $\mathbf{v}_{i}$ is the advective velocity at node $i$.  See KT Eqns (18), (19), (20), (25), (26) and (31).  The residual of the advection term at node $i$ is
\begin{equation}
R_{i} = -\sum_{j}k_{ij}u_{j} = -\sum_{j}k_{ij}(u_{j} - u_{i}) - u_{i}\sum_{j}k_{ij} \ .
\end{equation}
See KT Eqn (31).

$k_{ij}$ is evaluated for all pairs of nodes that are linked by the mesh.  Therefore, it may be evaluated by looping over elements, and adding contributions to the appropriate $i$-$j$ entries.  This is done by the [`AdvectiveFluxCalculatorConstantVelocity`](AdvectiveFluxCalculatorConstantVelocity.md) `UserObject`.

Let us evaluate $K$ for the initial configuration of our test example.  Each element yields
\begin{equation}
k^{\mathrm{ele}} = \left(
\begin{array}{cc}
k^{\mathrm{ele}}_{ll} & k^{\mathrm{ele}}_{lr} \\
k^{\mathrm{ele}}_{rl} & k^{\mathrm{ele}}_{rr}
\end{array}
\right) =
\frac{v}{2} \left(
\begin{array}{cc}
-1 & -1 \\
1 & 1
\end{array}
\right) \ .
\end{equation}
In this equation, the $l$ and $r$ subscripts mean "node to the left" and "node to the right".  Summing up all such contributions yields
\begin{equation}
\label{eq:kk}
K = \frac{v}{2} \left(
\begin{array}{ccccccc}
1 & -1 \\
1 & 0 & -1     \\
  & 1 & 0 & -1 \\
  &   & \ddots \\
  &   &   &   & 1 & 0 & -1 \\
  &   &   &   &   & 1 & -1
\end{array}
\right) \ .
\end{equation}
The top-left entry and the bottom-right entry may be safely ignored in the subsequent presentation, as they will be influenced by any boundary conditions that are on the left and right sides, respectively.

Inserting [eq:kk] into [eq:discadv] leads to the following standard central-difference representation of the advection equation:
\begin{equation}
\label{eqn:discadvcentdiff}
m_{i}\dot{u}_{i} = \frac{v}{2}(u_{i-1} - u_{i+1}) \ .
\end{equation}
(The equation will be different at the mesh boundaries, depending on the boundary conditions.)

Now consider the node at the position of the front (at $x=0.3$).  It has $u_{i-1}=1$, while $u_{i+1}=0$.  This means that $\dot{u}_{i}>0$.  For implicit timestepping, the explicit solution of [eqn:discadvcentdiff] is not immediately obvious, but nonetheless, the crucial point remains: spurious overshoots and undershoots appear in the solution, as shown pictorially in [fltvd_k_only].

!media media/porous_flow/fltvd_k_only.png style=width:60%;margin-left:10px caption=Temperature profile after one timestep, when only $K$ is used to transport the heat.  Notice the overshoots and undershoots.  id=fltvd_k_only

So far, the presentation has invoved only the standard Galerkin finite element scheme, with no special input from KT (other than their notation and referring to their formulae).  Now let us slowly introduce KT's design.

## Stabilization using upwinding

To remove the spurious oscillations that result from using $K$ to transport $u$, KT note that it is the negative off-diagonal entries of $K$ that are problematic.  In our example this is clear: we want to remove them so the discretised central-difference equation [eqn:discadvcentdiff] becomes a *full upwind* equation:
\begin{equation}
\label{eq:fullupwind}
m_{i}\dot{u}_{i} = v(u_{i-1} - u_{i}) \ .
\end{equation}
Here and below it is assumed $v>0$, otherwise the upwinding goes the opposite way, that is, from node $i+1$ to node $i$.

KT eliminate these negative off-diagonal entries in the following way.  Introduce the symmetric matrix $D$, with entries
\begin{equation}
d_{ij} = d_{ji} = \mathrm{max}\left\{ 0, -k_{ij}, -k_{ji} \right\}
\ \ \mathrm{and}\ \
d_{ii} = -\sum_{j\neq i}d_{ij} \ .
\end{equation}
(KT Eqn (32)).  So:

- $D$ picks out the negative off-diagonal entries of $K$, so it contains the entries that we want to elimiate
- $D$ is diagonal and each row of $D$ sums to zero, hence $D$ is a discrete diffusion operator

In our case
\begin{equation}
\label{eq:dd}
D = \frac{v}{2} \left(
\begin{array}{ccccccc}
-1 & 1 \\
1 & -2 & 1     \\
  & 1 & -2 & 1 \\
  &   & \ddots \\
  &   &   &   & 1 & -2 & 1 \\
  &   &   &   &   & 1 & -1
\end{array}
\right) \ .
\end{equation}

Then, instead of $K$, the operator $L = K + D$ is used to transport $u$:
\begin{equation}
m_{i}\dot{u}_{i} = \sum_{j}l_{ij}u_{j} \ .
\end{equation}
In our case
\begin{equation}
\label{eq:ll}
L = \frac{v}{2} \left(
\begin{array}{ccccccc}
0 & 0 \\
2 & -2 & 0     \\
  & 2 & -2 & 0 \\
  &   & \ddots \\
  &   &   &   & 2 & -2 & 0 \\
  &   &   &   &   & 2 & -2
\end{array}
\right) \ .
\end{equation}
(Once again, the top-left and bottom-right entries are potentially modified by boundary conditions.)  The full-upwind equation, [eq:fullupwind], is evidently obtained.

KT demonstrate that this process produces an $L$ that is Locally Extremum Diminishing, ie, that it never increases oscillations in the temperature profile.  Unfortunately, the process of employing $L$ instead of $K$ has (by design) added diffusion ($D$), and this is evident in the results, see [fltvd_l].

!media media/porous_flow/fltvd_l.png style=width:60%;margin-left:10px caption=Temperature profile after one timestep, when operator $L$ is used to transport the heat.  There are no overshoots and undershoots, but the sharp profile has diffused.  id=fltvd_l

## Flux-limiting and anti-diffusion

It is good to add diffusion ($D$) around the front position, because it prevents overshoots and undershoots, but elsewhere it is disasterous because it results in unphysical diffusion.  Therefore, KT add some anti-diffusion in regions where $D$ is not needed, to counter the effect of $D$.  KT introduce $P$ and $Q$ (Eqns (46), (47) and (48)) defined by
\begin{equation}
P_{i} = \sum_{j\neq i}\mathrm{min}\left\{0, k_{ij}\right\} (u_{j} - u_{i})
\end{equation}
and
\begin{equation}
Q_{i} = \sum_{j\neq i}\mathrm{max}\left\{0, k_{ij}\right\} (u_{j} - u_{i})
\end{equation}
In our case, using [eq:kk], these are
\begin{equation}
P_{i} = -\frac{v}{2}(u_{i+1} - u_{i})
\end{equation}
and
\begin{equation}
Q_{i} = \frac{v}{2}(u_{i-1} - u_{i})
\end{equation}
Note that $P$ concerns the negative entries of $K$, so these are the bits that are removed by adding diffusion $D$.  On the other hand, $Q$ is harmless: these parts do not introduce any overshoots or undershoots.  KT argue that looking at the ratio $Q/P$ should enable us to determine how much of $P$ really needed to be eliminated.

To this end, KT split $P$ and $Q$ into their positive and negative parts, and limit these parts separately (see Eqns (47), (48), (49) and the discussion of flux limiting on pp 134--135).  In our simple situation, there are four distinct cases, that are explicitly worked out in [kt_choices].  In working through this table, note that the end goal is $f_{i,i+1}^{a}$ (defined in KT Eqn (50)), which is the antidiffusive flux travelling from downwind node $i+1$ to the upwind node $i$.  Its purpose is to counter the diffusion we added in the $D$ matrix.

!table id=kt_choices caption=Possible choices for u around node $i$ and the consequences for the antidiffusion.  A "?" indicates the quantity is ill-defined due to a division $0/0$, but that the quantity does not matter.  In each case node $i+1$ is downwind of node $i$.
| | $u_{i-1}<u_{i}$ and $u_{i}>u_{i+1}$ | $u_{i-1}<u_{i}$ and $u_{i}<u_{i+1}$ | $u_{i-1}>u_{i}$ and $u_{i}<u_{i+1}$ | $u_{i-1}>u_{i}$ and $u_{i}>u_{i+1}$ |
| --- | --- | --- | --- | --- |
| | $u_{i}$ at maximum | $u$ increasing downstream | $u_{i}$ at minimum | $u$ decreasing downstream |
| $Q_{i}^{+}$ | 0 | 0 | $(u_{i-1}-u_{i})v/2>0$ | $(u_{i-1}-u_{i})v/2>0$ |
| $Q_{i}^{-}$ | $(u_{i-1}-u_{i})v/2<0$ | $(u_{i-1}-u_{i})v/2<0$ | 0 | 0 |
| $P_{i}^{+}$ | $(u_{i}-u_{i+1})v/2>0$ | 0 | 0 | $(u_{i}-u_{i+1})v/2>0$ |
| $P_{i}^{-}$ | 0 | $(u_{i}-u_{i+1})v/2<0$ | $(u_{i}-u_{i+1})v/2<0$ | 0 |
| $R_{i}^{+}$ | 0 | ? | ? | $\Phi_{i}=\Phi(\frac{u_{i}-u_{i-1}}{u_{i+1}-u_{i}})$ |
| $R_{i}^{-}$ | ? | $\Phi_{i}=\Phi(\frac{u_{i}-u_{i-1}}{u_{i+1}-u_{i}})$ | 0 | ? |
| $f_{i,i+1}^{a}$ | 0 (by $R^{+}$) | $\mathrm{min}(\Phi_{i} v/2, v)(u_{i}-u_{i+1})$ | 0 (by $R^{-}$) | $\mathrm{min}(\Phi_{i} v/2, v)(u_{i}-u_{i+1})$ |

The antidiffusive flux is zero when node $i$ is at a maximum or a minimum.  This is physically motivated by the "LED" criteria: we don't want local extrema to become more extreme: instead, we want to smooth them away with diffusion.  However, the antidiffusive flux is nonzero when the values of $u$ around node $i$ are such that $u$ is simply increasing or decreasing.  In these cases the addition of diffusion is not necessary, and the antidiffusion counters $D$ (to some extent, depending on $\Phi$).

The controlling factor is the flux limiter, $\Phi$.  Note that it is a function of the surrounding values of $u$:
\begin{equation}
\Phi_{i} = \Phi\left( \frac{u_{i}-u_{i-1}}{u_{i+1}-u_{i}} \right) \ .
\end{equation}
This is described in KT Figure 1.  Note that it is exactly the flux-limiter used in RDG(P0P1) (see the [RDG page](rdg/index.md optional=True)), so identical results to RDG(P0P1) should be expected in simple situations.

It is possible to choose all sorts of functional forms for $\Phi$.  KT write 4 possibilities on page 135.  All these possibilities satisfy $0\leq \Phi \leq 2$.  This means that the antidiffusive flux will always be
\begin{equation}
f_{i,i+1}^{a} = \Phi_{i} v (u_{i} - u_{i+1}) / 2
\end{equation}
With this antidiffusive flux, the evolution equation (see Step 3 of KT's Fig 2, along with Eqn(36) and the final expression in Eqn(50)) is just
\begin{equation}
m_{i}\dot{u}_{i} = v\left( u_{i-1}-u_{i} + \Phi_{i}(u_{i} - u_{i+1}) / 2 - \Phi_{i-1}(u_{i-1} - u_{i}) / 2\right)
\end{equation}
This is the final result of KT's procedure.

When $\Phi=0$, KT's procedure yields the *full upwind* equation [eq:fullupwind].  When $\Phi=1$, KT's procedure yields the *central difference* equation [eqn:discadvcentdiff].  When $\Phi=2$, KT's procedure yields the *downwind* equation $m_{i}\dot{u}_{i} = v(u_{i} - u_{i+1})$.  In general practice, KT's procedure produces no spurious overshoots or undershoots while strongly limiting artificial numerical diffusion.  The result for our test case is shown in [fltvd_all]

!media media/porous_flow/fltvd_all.png style=width:60%;margin-left:10px caption=Temperature profile after 1000 timesteps, when operator $L$ is used to transport the heat, and when $L$ plus antidiffusion is used.  id=fltvd_all


## Flux limiters

KT present four different flux-limiters.  These are plotted in [flux_liiters].

Notice that the VanLeer limiter is smoother than the others, so it is likely to provide superior nonlinear concergence, and it is chosen as the default limiter in the [AdvectiveFluxCalculatorConstantVelocity](AdvectiveFluxCalculatorConstantVelocity.md).

!media media/porous_flow/flux_limiters.png style=width:60%;margin-left:10px caption=Flux limiters, $\Phi(r)$, enumerated by KT (pp 135).  id=flux_liiters

## Effect of different flux limiters

The effect of choosing different flux limiters for this worked example is shown in [flux_limiter_choice].  In this case, the superbee flux limiter performs best.  Note that the timestep is rather large in this example, to emphasize the differences between the flux limiters, however, if this were a real simulation that was trying to reduce numerical diffusion, the timestep should be chosen smaller, and then the results would look more like [fltvd_all].

!media porous_flow/flux_limiter_choice.gif style=width:70%;margin-left:10px caption=The effect of flux limiter on the advective profile.  Note that the timestep is rather large here, to emphasise the difference between the flux limiters.  If a smaller timestep is chosen the results all look similar and like [fltvd_all].  id=flux_limiter_choice
