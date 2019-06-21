# Analytical Solution for a 1D equilibrium interface

!media media/phase_field/kks_c.png
       style=width:200px;margin-left:20px;float:right;
       caption=Composition $c$ for 2D simulation domain.

!media media/phase_field/kks_example_split_eta_0050.png
       style=width:200px;margin-left:20px;float:right;
       caption=Order parameter $\eta$ along $y = 0$.

!media media/phase_field/kks_example_split_c_0050.png
       style=width:200px;margin-left:20px;float:right;clear:both;
       caption=Composition $c$ along $y =0$.

In the KKS model, an analytical solution exists for the order parameter $\eta$ and composition $c$
through a 1D equilibrium interface:

\begin{equation}
\eta_0(x) = \frac{1}{2} \left[1-\tanh{\left( \frac{\sqrt{w}}{\sqrt{2} \epsilon} x\right)} \right]
\end{equation}

\begin{equation}
c_0(x) =  h(\eta_0(x))c_S^e + [1-h(\eta_0(x))]c_L^e
\end{equation}

where we use the switching function $h(\eta) = \eta^3(6\eta^2-15\eta+10)$, the
gradient energy coefficient $\epsilon^2 = 1$ and the barrier function height $w=1$.
(Note that there is a typo in Eq. (49) of [!cite](kim_phase-field_1999), $\epsilon$ should be in the
denominator of the argument to the $\tanh$ function, not $\sqrt{\epsilon}$.) In
the following example, we equilibrate a flat interface between a solid phase
($\eta = 1$, $f^S = (c_S-c_S^e)^2$, $c_S^e = 0.9$) and a liquid phase
($\eta = 0$, $f^L = (c_L-c_L^e)^2$, $c_L^e = 0.1$) in a 2D simulation. The vector
postprocessor `LineValueSampler` is used to obtain the values of $\eta$ and $c$
along $y=0$, the results are output to a CSV file, and plotted together with the
1D analytical solution. (We will use no-flux boundary conditions, so a boundary
conditions block is not required in the input file.)

!listing modules/phase_field/examples/kim-kim-suzuki/kks_example_noflux.i

# Verification against analytical solution

!media media/phase_field/kks_convergence.png
       style=width:200px;margin-left:20px;float:right;clear:both;
       caption=$L^2$ error for order parameter $\eta$.

To perform a more quantitative comparison of the simulation results to the analytical solution, we
will calculate the $L^2$ norm of the difference between the simulation result and the analytical
solution. The $L^2$ norm is defined for this case as

\begin{equation}
\left|u - u_h\right|_{L^2} = \left( \int_\Omega (u - u_h)^2 d\Omega \right)^{1/2}
\end{equation}
where $u$ is the analytical solution, $u_h$ is the equilibrium solution from simulation, and $\Omega$
is the domain. The $L^2$ norm can be obtained in the MOOSE framework using the
[`ElementL2Error`](/ElementL2Error.md) postprocessor. It can be shown from the
properties of the finite element method that for the linear Lagrange elements used in the split
formulation,

\begin{equation}
|u - u_h|_{L^2} \le Ch^2
\end{equation}
where $h$ is the characteristic element size (for 2D square elements, $h$ is the length of one side)
and $C$ is a constant specific to the problem. Thus, as the mesh is refined, $L^2$ error should
decrease (at least) quadratically with $h$.

In performing this comparison between the analytical solution and simulation results, if a no-flux
boundary condition is used in the simulation, the order parameter and composition profiles may shift
slightly in the $+x$ or $-x$ direction, even if the analytical solution is used as an initial
condition. This makes it difficult to compare to an analytical solution centered at $x=0$. Therefore,
we simulate the $x \ge 0$ half of the domain and use a Dirichlet boundary condition of $\eta=0.5$ and
$c=0.5$ at $x=0$, which prevents the interface from moving. For the verification, the size of the
domain was also reduced to lower the computational cost.

To verify that $L^2$ error decreases quadratically with $h$, see the plot showing $L^2$ error versus
$h$ for $\eta$ in this problem. As expected, on a log-log scale, the points are fit well by a
straight line. The slope was determined to be 1.995, in good agreement with the expected value of 2.

!listing modules/phase_field/examples/kim-kim-suzuki/kks_example_dirichlet.i
