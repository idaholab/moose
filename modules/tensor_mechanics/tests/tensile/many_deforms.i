# apply a number of "random" configurations and
# check that the algorithm returns to the yield surface
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin = -0.5
  xmax = 0.5
  ymin = -0.5
  ymax = 0.5
  zmin = -0.5
  zmax = 0.5
[]


[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[TensorMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]
[]


[BCs]
  [./bottomx]
    type = PresetBC
    variable = disp_x
    boundary = back
    value = 0.0
  [../]
  [./bottomy]
    type = PresetBC
    variable = disp_y
    boundary = back
    value = 0.0
  [../]
  [./bottomz]
    type = PresetBC
    variable = disp_z
    boundary = back
    value = 0.0
  [../]

  [./topx]
    type = FunctionPresetBC
    variable = disp_x
    boundary = front
    function = '(sin(0.05*t)+x)/1E0'
  [../]
  [./topy]
    type = FunctionPresetBC
    variable = disp_y
    boundary = front
    function = '(cos(0.04*t)+x*y)/1E0'
  [../]
  [./topz]
    type = FunctionPresetBC
    variable = disp_z
    boundary = front
    function = 't/1E2'
  [../]
[]

[AuxVariables]
  [./yield_fcn]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./yield_fcn_auxk]
    type = MaterialRealAux
    property = tensile_yield_function
    variable = yield_fcn
  [../]
[]

[Postprocessors]
  [./yield_fcn_at_zero]
    type = PointValue
    point = '0 0 0'
    variable = yield_fcn
    outputs = 'console'
  [../]
  [./should_be_zero]
    type = PlotFunction
    function = should_be_zero_fcn
  [../]
[]

[Functions]
  [./should_be_zero_fcn]
    type = ParsedFunction
    value = 'if(a<1E-3,0,a)'
    vars = 'a'
    vals = 'yield_fcn_at_zero'
  [../]
[]

[Materials]
  [./mc]
    type = FiniteStrainTensile
    block = 0
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    tensile_strength = 1.0
    tensile_tip_smoother = 0.5
    fill_method = symmetric_isotropic
    C_ijkl = '0 2.0E6'
    max_NR_iterations = 1000
    yield_function_tolerance = 1E-3
    ep_plastic_tolerance = 1E-6
    internal_constraint_tolerance = 1E-6
    debug_fspb = 1
  [../]
[]


[Executioner]
  end_time = 1E3
  dt = 1
  type = Transient
[]


[Outputs]
  file_base = many_deforms
  output_initial = true
  exodus = false
  [./console]
    type = Console
    perf_log = true
    linear_residuals = false
  [../]
  [./csv]
    type = CSV
    interval = 1
  [../]
[]
