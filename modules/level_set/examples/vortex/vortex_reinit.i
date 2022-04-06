[Mesh]
  type = GeneratedMesh
  dim = 2
  xmax = 1
  ymax = 1
  nx = 16
  ny = 16
  uniform_refine = 2
  elem_type = QUAD9
  second_order = true
[]

[AuxVariables]
  [./velocity]
    family = LAGRANGE_VEC
  [../]
[]

[AuxKernels]
  [./vec]
    type = VectorFunctionAux
    variable = velocity
    function = velocity_func
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
[]

[Variables]
  [phi]
    family = LAGRANGE
  []
[]

[Functions]
  [phi_exact]
    type = LevelSetOlssonBubble
    epsilon = 0.03
    center = '0.5 0.75 0'
    radius = 0.15
  []
  [./velocity_func]
    type = LevelSetOlssonVortex
    reverse_time = 2
  [../]
[]

[ICs]
  [phi_ic]
    type = FunctionIC
    function = phi_exact
    variable = phi
  []
[]

[Kernels]
  [time]
    type = TimeDerivative
    variable = phi
  []
  [advection]
    type = LevelSetAdvection
    velocity = velocity
    variable = phi
  []
  [advection_supg]
    type = LevelSetAdvectionSUPG
    velocity = velocity
    variable = phi
  []
  [time_supg]
    type = LevelSetTimeDerivativeSUPG
    velocity = velocity
    variable = phi
  []
[]

[Postprocessors]
  [area]
    type = LevelSetVolume
    threshold = 0.5
    variable = phi
    location = outside
    execute_on = 'initial timestep_end'
  []
  [cfl]
    type = LevelSetCFLCondition
    velocity = velocity
    execute_on = 'initial timestep_end'
  []
[]

[Problem]
  type = LevelSetProblem
[]

[Preconditioning/smp]
    type = SMP
    full = true
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  start_time = 0
  end_time = 2
  scheme = crank-nicolson
  [TimeStepper]
    type = PostprocessorDT
    postprocessor = cfl
    scale = 0.8
  []
[]

[MultiApps]
  [reinit]
    type = LevelSetReinitializationMultiApp
    input_files = 'vortex_reinit_sub.i'
    execute_on = TIMESTEP_END
  []
[]

[Transfers]
  [to_sub]
    type = MultiAppCopyTransfer
    source_variable = phi
    variable = phi
    to_multi_app = reinit
    execute_on = 'timestep_end'
  []

  [to_sub_init]
    type = MultiAppCopyTransfer
    source_variable = phi
    variable = phi_0
    to_multi_app = reinit
    execute_on = 'timestep_end'
  []

  [from_sub]
    type = MultiAppCopyTransfer
    source_variable = phi
    variable = phi
    from_multi_app = reinit
    execute_on = 'timestep_end'
  []
[]

[Outputs]
  csv = true
  exodus = true
[]
