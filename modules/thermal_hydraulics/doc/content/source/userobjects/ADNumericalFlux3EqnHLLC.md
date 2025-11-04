# ADNumericalFlux3EqnHLLC

This class implements the HLLC Riemann solver for the 1-D, variable-area Euler
equations, which computes a numerical flux from two solution values:
\begin{equation}
  \mathbf{F} = \mathcal{F}(\mathbf{U}_1, \mathbf{U}_2, \mathbf{n}) \,,
\end{equation}
where $\mathbf{n}$ is the normal unit vector in the direction of cell 2 from
cell 1. This implementation is based on the work by
[!cite](batten1997average), but the equation of state is generalized instead of
assuming ideal gas.

The parameter [!param](/UserObjects/ADNumericalFlux3EqnHLLC/wave_speed_formulation)
gives the choice of the formulation for the left and right wave speeds:

- `einfeldt` (the default) [!citep](einfeldt1991):

  !equation
  s_L = \text{min}(u_L - c_L, \tilde{u} - \tilde{c})

  !equation
  s_R = \text{max}(u_R + c_R, \tilde{u} + \tilde{c})

  !equation
  \tilde{u} = \frac{\sqrt{\rho_L}u_L + \sqrt{\rho_R}u_R}
    {\sqrt{\rho_L} + \sqrt{\rho_R}}

  !equation
  \tilde{c} = c(\tilde{v}, \tilde{h})

  !equation
  \tilde{v} = \frac{1}{\tilde{\rho}}

  !equation
  \tilde{\rho} = \sqrt{\rho_L \rho_R}

  !equation
  \tilde{h} = \tilde{H} - \frac{1}{2}\tilde{u}^2

  !equation
  \tilde{H} = \frac{\sqrt{\rho_L}H_L + \sqrt{\rho_R}H_R}
    {\sqrt{\rho_L} + \sqrt{\rho_R}}

- `davis` [!citep](davis1988):

  !equation
  s_L = \text{min}(u_L - c_L, u_R - c_R)

  !equation
  s_R = \text{max}(u_L + c_L, u_R + c_R)

!syntax parameters /UserObjects/ADNumericalFlux3EqnHLLC

!syntax inputs /UserObjects/ADNumericalFlux3EqnHLLC

!syntax children /UserObjects/ADNumericalFlux3EqnHLLC

!bibtex bibliography
