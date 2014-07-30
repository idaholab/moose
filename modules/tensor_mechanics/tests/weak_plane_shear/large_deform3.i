# apply a number of "random" configurations and
# check that the algorithm returns to the yield surface
#
# must be careful here - we cannot put in arbitrary values of C_ijkl, otherwise the condition
# df/dsigma * C * flow_dirn < 0 for some stresses
# The important features that must be obeyed are:
# 0 = C_0222 = C_1222  (holds for transversely isotropic, for instance)
# C_0212 < C_0202 = C_1212 (holds for transversely isotropic)

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

  # the following are "random" deformations
  # each is O(1E-1) to provide large deformations
  [./topx]
    type = FunctionPresetBC
    variable = disp_x
    boundary = front
    function = '(sin(0.1*t)+x)/1E1'
  [../]
  [./topy]
    type = FunctionPresetBC
    variable = disp_y
    boundary = front
    function = '(cos(t)+x*y)/1E1'
  [../]
  [./topz]
    type = FunctionPresetBC
    variable = disp_z
    boundary = front
    function = 'sin(0.4321*t)*x*y*z/1E1'
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
    property = weak_plane_shear_yield_function
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
    type = FiniteStrainWeakPlaneShear
    block = 0
    wps_cohesion = 1E3
    wps_dilation_angle = 5
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    # the following is transversely isotropic, i think.
    fill_method = symmetric9
    C_ijkl = '3E9 1E9 3E9 3E9 3E9 6E9 1E9 1E9 9E9'
    wps_friction_angle = 30
    wps_normal_vector = '0 0 1'
    wps_normal_rotates = true
    wps_smoother = 100
    max_NR_iterations = 100
    yield_function_tolerance = 1E-3
    ep_plastic_tolerance = 1E-3
    internal_constraint_tolerance = 1E-3
    debug_fspb = 1
  [../]
[]


[Executioner]
  end_time = 1E4
  dt = 1
  type = Transient
[]


[Outputs]
  file_base = large_deform3
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
