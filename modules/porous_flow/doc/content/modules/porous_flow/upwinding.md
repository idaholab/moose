# Full upwinding

This page is part of a set of pages devoted to discussions of numerical stabilization in PorousFlow.  See:

- [Numerical stabilization lead page](stabilization.md)
- [Mass lumping](mass_lumping.md)
- [Full upwinding](upwinding.md)
- [Kuzmin-Turek stabilization](kt.md)
- [Numerical diffusion](numerical_diffusion.md)
- [A worked example of Kuzmin-Turek stabilization](kt_worked.md)

Numerical stabilization is necessary in scenarios with nonlinear advection, such as the physics modelled by PorousFlow.  One such stabilization is upwinding [!citep](huyakorn1978, Dalen1979, helmig1998). For multi-phase situations many upwinding schemes can lead to disaster as the algorithm attempts to withdraw fluid from a node where there is no fluid.  In situations where one phase disappears, or almost disappears, full upwinding is often advantageous, since it prevents such attempted withdrawls, and hence PorousFlow employs full upwinding by default.

The [`PorousFlowBasicAdvection`](PorousFlowBasicAdvection.md) kernel
implements a very naive non-upwinded version of advection.  Its use is
not recommended.

Full upwinding is numerically cheap compared with other stabilization schemes, but it suffers from two disadvantages.  Firstly, it is not smooth (in contrast
to the SUPG upwinding scheme [!citep](brooks1982, hughes1986, hughes1986b) or the [Kuzmin-Turek scheme](kt.md) [!citep](KuzminTurek2004), for instance), so that close to steady-state where the fluxes are zero, the
upwind direction can oscillate, leading to nonconvergence.  This is dealt
with by placing a cutoff on the upwinding in PorousFlow.  Secondly, upwinding is adding numerical diffusion, which can be critical in some simulations: see [numerical diffusion](numerical_diffusion.md) for a discussion.  PorousFlow also offers the [Kuzmin-Turek stabilization scheme](kt.md) for users who are concerned with these disadvantages.

The remainder of this
section describes full upwinding for the single-phase unsaturated situation.
The multi-phase, multi-component scenario, and the advective term in the
heat-flow equation are analogous.  

The weak form of the Darcy flux of for a single element is
\begin{equation}
R_{a} = \int_{\mathrm{element}} \nabla_{i}\psi_{a}
\frac{\rho k_{ij}\,k_{\mathrm{r}}}{\mu}(\nabla_{j}P - \rho
g_{j})  \ .
\end{equation}

Here $\psi_{a}$ is the test function that we are integrating against, and
$R_{a}$ is the contribution to the residual for this test function.  Define

\begin{equation}
m = \frac{\rho k_{\mathrm{r}}}{\mu} \ ,
\end{equation}
which is traditionally called the mobility.

Upwinding is all a matter of choosing where in the element to evaluate $m$ in
the above integral.  The sophisticated SUPG approach was designed to weight $m$
on the upstream side of the element.  Consider for a moment taking $m$ outside
the integral:

\begin{equation}
\tilde{R}_{a} = m\int_{\mathrm{element}} \nabla_{i}\psi_{a}
\kappa_{ij}(\nabla_{j}P - \rho
g_{j}) = m I_{a} \ .
\end{equation}
$\tilde{R}$ is not exactly $R$, but note:

- the original $R_{a}$ is the mass flux flowing out of node $a$;
- so $I_{a}$ is thereby intepreted as a measure of fluid flow out of
  node $a$.

This leads to the following definition of upwinding:
\begin{equation}
R_{a} \equiv m_{a}I_{a} \ \ \ \textrm{if}\ \ \ I_{a}\geq 0 \ .
\end{equation}

The nodes for which $I_{a}\geq 0$ are called *upwind nodes*, since fluid is
flowing from them to the *downwind nodes*.  This approach was first introduced
by [!cite](dalen1979).

The residual at the downwind nodes is determined by conserving mass.
Specifically, let
\begin{equation}
I_{\mathrm{up}} = \sum_{I_{a}\geq 0}I_{a} \ \ \ \textrm{and}\ \ \
I_{\mathrm{down}} = -\sum_{I_{a}<0} I_{a} \ .
\end{equation}
Then
\begin{equation}
R_{a} = I_{a}\frac{I_{\mathrm{up}}}{I_{\mathrm{down}}}
\ \ \ \textrm{for}\ \ \ I_{a}<0 \ .
\end{equation}
Then $\sum_{a} R_{a} = 0$ as required by mass conservation within the element (which originates from $\sum_{a} \psi_{a} = 1$).

The fully-upwind method is extremely advantageous to use if fluid saturations
ever approach residual saturation (where $\kappa_{\mathrm{rel}}=0$) or zero
density, for then the mobility is zero and it becomes impossible to withdraw
fluid from such a node (in practice this may still happen due to precision loss
or other related nasty artefacts).

Prescribed sinks, either from the boundary or from internal objects such as
wellbores, are also fully-upwinded in PorousFlow since they also potentially
suffer from phase-disappearence problems.


!bibtex bibliography
