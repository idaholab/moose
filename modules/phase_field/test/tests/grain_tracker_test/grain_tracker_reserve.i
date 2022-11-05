[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 40
  ny = 40
  xmin = 0
  xmax = 100
  ymin = 0
  ymax = 100
  elem_type = QUAD4
[]

[AuxVariables]
  [./c]
  [../]
[]

[Variables]
  [./gr0]
  [../]
  [./gr1]
  [../]
[]

[ICs]
  [./gr0]
    type = MultiSmoothCircleIC
    variable = gr0
    invalue = 1.0
    outvalue = 0.0001
    bubspac = 20.0
    numbub = 2
    radius = 10.0
    int_width = 12.0
    radius_variation = 0.2
    radius_variation_type = uniform
  [../]
  [./c_IC]
    type = SmoothCircleIC
    int_width = 12.0
    x1 = 50
    y1 = 50
    radius = 10.0
    outvalue = 0
    variable = c
    invalue = 1
  [../]
[]

[Kernels]
  [./ie_gr0]
    type = TimeDerivative
    variable = gr0
  [../]
  [./diff_gr0]
    type = Diffusion
    variable = gr0
  [../]
  [./ie_gr1]
    type = TimeDerivative
    variable = gr1
  [../]
  [./diff_gr1]
    type = Diffusion
    variable = gr1
  [../]
  [./source]
    type = MaskedBodyForce
    variable = gr1
    function = t
    mask = mask
  [../]
[]

[Materials]
  [./mask]
    type = ParsedMaterial
    expression = 'c'
    property_name = mask
    coupled_variables = 'c'
  [../]
[]

[Postprocessors]
  [./grain_tracker]
    type = GrainTracker

    # Reserve the first "op" variable
    reserve_op = 1
    threshold = 0.1
    connecting_threshold = 0.001
    variable = 'gr0 gr1'
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  num_steps = 6
  dt = 0.25
[]

[Outputs]
  exodus = true
[]

[Problem]
  kernel_coverage_check = false
[]
