# Transient diffusion with a nodal constraint. The constraint residual is part of the non-time
# residual, which the multistage integrators weight with their stage coefficients, so the
# constraint Jacobian has to be weighted as well.
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
  allow_renumbering = false
[]
[Variables]
  [u]
  []
[]
[ICs]
  [u]
    type = FunctionIC
    variable = u
    function = 'x*(1-x) + y'
  []
[]
[Kernels]
  [time]
    type = TimeDerivative
    variable = u
  []
  [diff]
    type = Diffusion
    variable = u
  []
[]
[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 1
  []
[]
[Constraints]
  [top]
    type = EqualValueBoundaryConstraint
    variable = u
    secondary = 'top'
    penalty = 10
  []
[]
[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]
[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 1
  dt = 0.1
  [TimeIntegrator]
    type = ImplicitMidpoint
  []
[]
