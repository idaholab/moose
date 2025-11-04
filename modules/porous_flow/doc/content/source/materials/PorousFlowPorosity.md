# PorousFlowPorosity

!syntax description /Materials/PorousFlowPorosity

This Material computes porosity (at the nodes or quadpoints, depending on the `at_nodes` flag):
\begin{equation}
\label{eq:poro_evolve}
\phi + M = \alpha_{B} + (\phi_{0} + M_{\mathrm{ref}} - \alpha_{B})\times \exp \left( \frac{\alpha_{B}'
  - 1}{K}(P_{f} - P_{f}^{\mathrm{ref}}) - \epsilon^{\mathrm{total}}_{ii} + \alpha_{T}(T - T^{\mathrm{ref}}) \right) \ ,
\end{equation}
A full description is provided in the [porosity documentation](/porous_flow/porosity.md)

Flags provided to `PorousFlowPorosity` control its evolution.

- If `mechanical = true` then the porosity will depend on $\epsilon^{\mathrm{total}}_{ii}$.
  Otherwise that term in [eq:poro_evolve] is ignored.

- If `fluid = true` then the porosity will depend on $(P_{f} - P_{f}^{\mathrm{ref}})$.  Otherwise
  that term in [eq:poro_evolve] is ignored.

- If `thermal = true` then the porosity will depend on $(T - T^{\mathrm{ref}})$.  Otherwise that term
  in [eq:poro_evolve] is ignored.

- If `chemical = true` then porosity will depend on $M$.  Otherwise that term in
  [eq:poro_evolve] is ignored.

!alert note
On the parameter `solid_bulk`: Selecting a functor with dependency on a model
variable $u$ (such as pore pressure) for the parameter `solid_bulk` $K$ in a way
that $\text{d}K / \text{d}u \neq 0$ without taking further precautions may result in an
inexact Jacobian matrix, which can lead to convergence problems. These problems
should not be triggered by a constant solid bulk or a pure dependency on the
model's time and/or spatial coordinates.

!syntax parameters /Materials/PorousFlowPorosity

!syntax inputs /Materials/PorousFlowPorosity

!syntax children /Materials/PorousFlowPorosity
