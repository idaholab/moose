[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Problem]
  linear_sys_names = 'u_sys'
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    solver_sys = 'u_sys'
  []
[]

[LinearFVKernels]
  [timeu]
    type = LinearFVTimeDerivative
    variable = u
  []
  [diffu]
    type = LinearFVDiffusion
    variable = u
  []
  [forceu]
    type = LinearFVSource
    variable = u
    source_density = force
  []
[]

[Functions]
  [exact]
    type = ParsedFunction
    expression = 't^3*x*y'
  []
  [force]
    type = ParsedFunction
    expression = '3*x*y*t^2'
  []
[]

[LinearFVBCs]
  [allu]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    functor = exact
    variable = u
    boundary = 'left right top bottom'
  []
[]

[Executioner]
  type = Transient
  system_names = u_sys
  l_tol = 1e-10
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'hypre'
  dt = 1
  end_time = 3
  scheme = 'bdf2'
[]

[Postprocessors]
  [L2u]
    type = ElementL2Error
    function = exact
    variable = u
  []
[]

[Outputs]
  csv = true
[]
