[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 2
  []
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

[AuxVariables]
  [v]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [v]
    type = KokkosCopyValueAux
    source = u
    variable = v
  []
[]

[LinearFVKernels]
  [diffusion]
    type = KokkosLinearFVDiffusion
    variable = u
    diffusion_coeff = 'unity'
  []
[]

[LinearFVBCs]
  [left]
    type = KokkosLinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = "left"
    diffusion_coeff = 'unity'
    functor = 'zero'
  []
  [right]
    type = KokkosLinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = "right"
    diffusion_coeff = 'unity'
    functor = 'unity'
  []
[]

[Functions]
  [zero]
    type = KokkosParsedFunction
    expression = '0'
  []
  [unity]
    type = KokkosParsedFunction
    expression = '1'
  []
[]

[Postprocessors]
  [kokkos_avg]
    type = KokkosElementAverageValue
    variable = v
    execute_on = FINAL
  []
  [linear_fv_avg]
    type = ElementFVAverageValue
    functor = 'u'
    execute_on = FINAL
  []
[]

[Executioner]
  type = Steady
  system_names = u_sys
  l_tol = 1e-10
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = FINAL
  []
  [console]
    type = Console
    execute_on = FINAL
  []
[]
