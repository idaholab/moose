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
    type = KokkosLinearFVFunctorDirichletBC
    variable = u
    boundary = "left"
    functor = 'zero'
  []
  [right]
    type = KokkosLinearFVFunctorDirichletBC
    variable = u
    boundary = "right"
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
    type = ElementAverageFunctorPostprocessor
    functor = 'u'
    execute_on = FINAL
    evaluation_type = CELL_AVERAGE
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
