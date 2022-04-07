[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 100
[]

[Variables]
  [v]
    type = MooseVariableFVReal
    initial_condition = 0
  []
[]

[AuxVariables]
  [dummy]
    type = MooseVariableFVReal
  []
[]

[FVKernels]
  [time]
    type = FVTimeKernel
    variable = v
  []
  [diff]
    type = FVDiffusion
    variable = v
    coeff = coeff
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    variable = v
    boundary = left
    value = 0
  []
  [right]
    type = FVDirichletBC
    variable = v
    boundary = right
    value = 1
  []
[]

[Materials]
  [diff]
    type = ADGenericFunctorMaterial
    prop_names = 'coeff'
    prop_values = '1'
  []
[]

[Postprocessors]
  [average]
    type = ElementAverageValue
    variable = v
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e-6
    optimal_iterations = 6
  []
  end_time = 1000
  nl_abs_tol = 1e-8
[]

[Outputs]
  exodus = false
  [csv]
    type = CSV
    execute_on = 'final'
  []
[]

[Adaptivity]
  steps = 1
  marker = error
  [Indicators]
    [jump]
      type = GradientJumpIndicator
      variable = v
    []
  []
  [Markers]
    [error]
      type = ErrorFractionMarker
      coarsen = 0.1
      refine = 0.7
      indicator = jump
    []
  []
  max_h_level = 1
[]
