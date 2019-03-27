# Numerical stabilization

This page is part of a set of pages devoted to discussions of numerical stabilization in PorousFlow.  See:

- [Numerical stabilization lead page](stabilization.md)
- [Mass lumping](mass_lumping.md)
- [Full upwinding](upwinding.md)
- [Kuzmin-Turek stabilization](kt.md)
- [Numerical diffusion](numerical_diffusion.md)
- [A worked example of Kuzmin-Turek stabilization](kt_worked.md)

Numerical stabilization is often necessary in PorousFlow simulations.  There are a number of issues to consider when choosing an appropriate stabilization scheme.

- Expense of computing the residual and Jacobian.  Naively this dictates how fast MOOSE will run.  However, a cheap stabilization sometimes produces very poor [nonlinear convergence](nonlinear_convergence_problems.md), so a huge number of nonlinear iterations are required, offsetting the apparent cheapness.

- Smoothness.  Some schemes are "smoother" than others, meaning they converge at the same rate irrespective of the values of the Variables.  On the other hand, some schemes are not smooth.  For instance, the [full upwinding](upwinding.md) often has trouble converging when the system is close to steady state, because the scheme finds it difficult to know which nodes are "upstream" and which are "downstream" and keeps swapping nodes in and out of each category.  Hence, non-smooth schemes may converge very nicely for some simulations, but very poorly in others.

- Overshoots.  Advection often produces spurious overshoots and undershoots, and boundary terms and Dirac sources often attempt to withdraw fluid from regions where there is none.  Both of these lead to [physically-incorrect predictions](numerical_diffusion.md) and [nonlinear convergence problems](nonlinear_convergence_problems.md).

- [Numerical diffusion](numerical_diffusion.md).  Stabilization schemes often [add numerical diffusion](kt_worked.md) in order to eliminate the overshoots.  This means that precise tracking of tracers or temperature fronts is not possible.

At present, PorousFlow offers two types of numerical stabilization: [full upwinding](upwinding.md) and [Kuzmin-Turek stabilization](kt.md).  Both employ [mass lumping](mass_lumping.md).  The pros and cons of the types of stabilization are shown below

!table id=procon caption=Pros and Cons of the stabilization schemes
|  | No stabilization | Full upwinding | KT stabilization |
| --- | --- | --- | --- |
| Expense | [!style color=green](cheap) | [!style color=green](cheap) | [!style color=red](expensive) |
| Smooth | [!style color=green](yes) | [!style color=red](no) | [!style color=green](yes) |
| Overshoots | [!style color=red](yes) | [!style color=green](no) | [!style color=green](no) |
| Diffusion | [!style color=green](minimal) | [!style color=red](maximal) | [!style color=green](minimal) |

!alert note
The Kuzmin-Turek stabilization is new and users are strongly encouraged to experiment with it and report their findings to the moose-users google group to iron out any problems and so we can collectively gain experience.

