# Worked example of Kuzmin-Turek stabilization

Kuzmin and Turek [citep:KuzminTurek2004] describe a method of stabilising advection while minimising artificial numerical diffusion.  In this page "Kuzmin and Turek" is abbreviatved to "KT".  This page will make much more sense if you read it in tandem with the KT paper!  KT consider a single scalar quantity $u$ that is being advected.  For sake of argument, in this page we think of $u$ as "heat".

In this page, the 1D example studied in the [numerical diffusion page](numerical_diffusion.md) is used to explicitly illustrate how their approach works.  The input file is

!listing modules/porous_flow/test/tests/numerical_diffusion/fltvd.i

The mesh sits in the domain $0\leq x \leq 1$ and is meshed with 10 elements.  The initial condition is $u(x)=1$ if $0.1\leq x \leq 3$, and $u(x)=0$ otherwise.  The velocity is uniform to the right: $v=0.1$.

KT work with a lumped mass matrix (see discussion at the start of KT Section 5, Eqn (25)), so the input file uses the [MassLumpedTimeDerivative](framework/doc/content/source/kernels/MassLumpedTimeDerivative.md).

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

$k_{ij}$ is evaluated for all pairs of nodes that are linked by the mesh.  Therefore, it may be evaluated by looping over elements, and adding contributions to the appropriate $i$-$j$ entries.

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
  &   & \ldots \\
  &   &   &   & 1 & 0 & -1 \\
  &   &   &   &   & 1 & -1
\end{array}
\right) \ .
\end{equation}
The top-left entry and the bottom-right entry may be safely ignored in the subsequent presentation, as they will be influenced by any boundary conditions that are on the left and right sides, respectively.

Inserting [eq:kk] into [eq:discadv] leads to the following standard central-difference representation of the advection equation:
\begin{equation}
\label{eqn:discadvcentdiff}
m_{i}\dot_{u}_{i} = \frac{v}{2}(u_{i-1} - u_{i+1}) \ .
\end{equation}
(The equation will be different at the mesh boundaries, depending on the boundary conditions.)

Now consider the node at the position of the front (at $x=0.3$).  It has $u_{i-1}=1$, while $u_{i+1}=0$.  This means that $\dot_{u}_{i}>0$.  For implicit timestepping, the explicit solution of [eqn:discadvcentdiff] is not immediately obvious, but nonetheless, the crucial point remains: spurious overshoots and undershoots appear in the solution, as shown pictorially in [fltvd_k_only].

!media media/porous_flow/fltvd_k_only.png style=width:60%;margin-left:10px caption=Temperature profile after one timestep, when only $K$ is used to transport the heat.  Notice the overshoots and undershoots.  id=fltvd_k_only

So far, the presentation has invoved only the standard Galerkin finite element scheme, with no special input from KT (other than their notation and referring to their formulae).  Now let us slowly introduce KT's design.

## Stabilization using upwinding

To remove the spurious oscillations that result from using $K$ to transport $u$, KT note that it is the negative off-diagonal entries of $K$ that are problematic.  In our example this is clear: we want to remove them so the discretised central-difference equation [eqn:discadvcentdiff] becomes a *full upwind* equation:
\begin{equation}
\label{eq:fullupwind}
m_{i}\dot_{u}_{i} = v(u_{i-1} - u_{i}) \ .
\end{equation}
Here and below it is assumed $v>0$, otherwise the upwinding goes from node $i+1$ to node $i$.

KT eliminate these negative off-diagonal entries in the following way.  Introduce the symmetric matrix $D$, with entries
\begin{equation}
d_{ij} = d_{ji} = \mathrm{max}\left\{ 0, -k_{ij}, -k_{ji} \right\}
\ \ \
d_{ii} = -\sum_{j\neq i}d_{ij} \ .
\end{equation}
(KT Eqn (32)).  So:

- $D$ picks out the negative off-diagonal entries of $K$, so it contains the entries that we want to elimiate
- each row of $D$ sums to zero, hence $D$ is a discrete diffusion operator

In our case
\begin{equation}
\label{eq:dd}
D = \frac{v}{2} \left(
\begin{array}{ccccccc}
-1 & 1 \\
1 & -2 & 1     \\
  & 1 & -2 & 1 \\
  &   & \ldots \\
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
\label{eq:dd}
L = \frac{v}{2} \left(
\begin{array}{ccccccc}
0 & 0 \\
2 & -2 & 0     \\
  & 2 & -2 & 0 \\
  &   & \ldots \\
  &   &   &   & 2 & -2 & 0 \\
  &   &   &   &   & 2 & -2
\end{array}
\right) \ .
\end{equation}
(Once again, the top-left and bottom-right entries are potentially modified by boundary conditions.)  The full-upwind equation, [eq:fullupwind], is evidently obtained.

KT demonstrate that this process produces an $L$ that is Locally Extremum Diminishing, ie, that it never increases oscillations in the temperature profile.  Unfortunately, the process of employing $L$ instead of $K$ has (by design) added diffusion ($D$), and this is evident in the results, see [fltvd_l].

!media media/porous_flow/fltvd_l.png style=width:60%;margin-left:10px caption=Temperature profile after one timestep, when operator $L$ is used to transport the heat.  There are no overshoots and undershoots, but the sharp profile has diffused.  id=fltvd_l

## Flux-limiting and anti-diffusion

It is good to add diffusion ($D$) around the front position, because it prevents overshoots and undershoots, but elsewhere it is disasterous because it results in unphysical diffusion.  Therefore, KT add some anti-diffusion in regions where $D$ is not needed, to counter the effect of $D$.  KT introduce $P$ and $Q$, defined by
\begin{eqnarray}
P_{i} & = & \sum_{j\neq i}\mathrm{min}\left\{0, k_{ij}\right\} (u_{j} - u_{i}) \\
Q_{i} & = & \sum_{j\neq i}\mathrm{max}\left\{0, k_{ij}\right\} (u_{j} - u_{i})
\end{eqnarray}
In our case, using [eq:kk], these are
\begin{eqnarray}
P_{i} & = & -\frac{v}{2}(u_{i+1} - u_{i}) \\
Q_{i} & = & \frac{v}{2}(u_{i-1} - u_{i})
\end{eqnarray}
Note that $P$ concerns the negative entries of $K$, so these are the bits that are removed by adding diffusion $D$.  On the other hand, $Q$ is harmless: these parts do not introduce any overshoots or undershoots.  KT argue that looking at the ratio $Q/P$ should enable us to determine how much of $P$ really needed to be eliminated.


