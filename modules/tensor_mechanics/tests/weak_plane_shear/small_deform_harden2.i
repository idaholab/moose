# apply a pure tension, then some shear with compression
# the BCs are designed to map out the yield function, showing
# the affect of the hardening
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
  [./x_disp]
  [../]
  [./y_disp]
  [../]
  [./z_disp]
  [../]
[]

[TensorMechanics]
  [./solid]
    disp_x = x_disp
    disp_y = y_disp
    disp_z = z_disp
  [../]
[]


[BCs]
  [./bottomx]
    type = PresetBC
    variable = x_disp
    boundary = back
    value = 0.0
  [../]
  [./bottomy]
    type = PresetBC
    variable = y_disp
    boundary = back
    value = 0.0
  [../]
  [./bottomz]
    type = PresetBC
    variable = z_disp
    boundary = back
    value = 0.0
  [../]

  [./topx]
    type = FunctionPresetBC
    variable = x_disp
    boundary = front
    function = 'if(t<1E-6,0,3*t)'
  [../]
  [./topy]
    type = FunctionPresetBC
    variable = y_disp
    boundary = front
    function = 'if(t<1E-6,0,5*(t-0.01E-6))'
  [../]
  [./topz]
    type = FunctionPresetBC
    variable = z_disp
    boundary = front
    function = 'if(t<1E-6,t,2E-6-t)'
  [../]
[]

[AuxVariables]
  [./stress_xz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./wps_internal]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./yield_fcn]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./stress_xz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xz
    index_i = 0
    index_j = 2
  [../]
  [./stress_zx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zx
    index_i = 2
    index_j = 0
  [../]
  [./stress_yz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yz
    index_i = 1
    index_j = 2
  [../]
  [./stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 2
    index_j = 2
  [../]
  [./wps_internal_auxk]
    type = MaterialRealAux
    property = weak_plane_shear_internal
    variable = wps_internal
  [../]
  [./yield_fcn_auxk]
    type = MaterialRealAux
    property = weak_plane_shear_yield_function
    variable = yield_fcn
  [../]
[]

[Postprocessors]
  [./s_xz]
    type = PointValue
    point = '0 0 0'
    variable = stress_xz
  [../]
  [./s_yz]
    type = PointValue
    point = '0 0 0'
    variable = stress_yz
  [../]
  [./s_zz]
    type = PointValue
    point = '0 0 0'
    variable = stress_zz
  [../]
  [./f]
    type = PointValue
    point = '0 0 0'
    variable = yield_fcn
  [../]
  [./int]
    type = PointValue
    point = '0 0 0'
    variable = wps_internal
  [../]
[]

[Materials]
  [./mc]
    type = FiniteStrainWeakPlaneShear
    block = 0
    wps_cohesion = 1E3
    wps_cohesion_residual = 700
    wps_cohesion_rate = 1E8
    wps_dilation_angle = 5
    wps_dilation_angle_residual = 1
    wps_dilation_angle_rate = 1E8
    disp_x = x_disp
    disp_y = y_disp
    disp_z = z_disp
    fill_method = symmetric_isotropic
    C_ijkl = '1E9 0.5E9'
    wps_friction_angle = 45
    wps_friction_angle_residual = 30
    wps_friction_angle_rate = 1E8
    wps_normal_vector = '0 0 1'
    wps_normal_rotates = false
    wps_smoother = 500
    yield_function_tolerance = 1E-3
    ep_plastic_tolerance = 1E-3
    internal_constraint_tolerance = 1E-3
    debug_fspb = 1
  [../]
[]


[Executioner]
  end_time = 2E-6
  dt = 1E-7
  type = Transient
[]


[Outputs]
  file_base = small_deform_harden2
  output_initial = true
  exodus = true
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
