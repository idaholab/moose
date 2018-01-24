# apply a pure tension, then some shear
# the BCs are designed to map out the yield function, showing
# the affect of 'cap' smoothing
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

[Kernels]
  [./TensorMechanics]
    displacements = 'x_disp y_disp z_disp'
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
    function = 'if(t<1E-6,0,3*(t-1E-6)*(t-1E-6)*1E6)'
  [../]
  [./topy]
    type = FunctionPresetBC
    variable = y_disp
    boundary = front
    function = 'if(t<1E-6,0,5*(t-1E-6)*(t-1E-6)*1E6)'
  [../]
  [./topz]
    type = FunctionPresetBC
    variable = z_disp
    boundary = front
    function = 'if(t<1E-6,t,1E-6)'
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
  [./yield_fcn]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./iter]
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
  [./yield_fcn_auxk]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 0
    variable = yield_fcn
  [../]
  [./iter_auxk]
    type = MaterialRealAux
    property = plastic_NR_iterations
    variable = iter
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
  [./iter]
    type = PointValue
    point = '0 0 0'
    variable = iter
  [../]
[]

[UserObjects]
  [./coh]
    type = TensorMechanicsHardeningConstant
    value = 1E3
  [../]
  [./tanphi]
    type = TensorMechanicsHardeningConstant
    value = 1
  [../]
  [./tanpsi]
    type = TensorMechanicsHardeningConstant
    value = 0.08748866
  [../]
  [./wps]
    type = TensorMechanicsPlasticWeakPlaneShear
    cohesion = coh
    tan_friction_angle = tanphi
    tan_dilation_angle = tanpsi
    tip_scheme = cap
    smoother = 0
    cap_rate = 0.001
    cap_start = -1000.0
    yield_function_tolerance = 1E-3
    internal_constraint_tolerance = 1E-6
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    fill_method = symmetric_isotropic
    C_ijkl = '1E9 0.5E9'
  [../]
  [./strain]
    type = ComputeFiniteStrain
    block = 0
    displacements = 'x_disp y_disp z_disp'
  [../]
  [./mc]
    type = ComputeMultiPlasticityStress
    ep_plastic_tolerance = 1E-4
    block = 0
    plastic_models = wps
    transverse_direction = '0 0 1'
    debug_fspb = crash
    debug_jac_at_stress = '1E4 2E4 3E4 2E4 -4E4 5E4 3E4 5E4 6E8'
    debug_jac_at_pm = 1
    debug_jac_at_intnl = 1
    debug_stress_change = 1E-3
    debug_pm_change = 1E-5
    debug_intnl_change = 1E-5
  [../]
[]


[Executioner]
  end_time = 2E-6
  dt = 1E-7
  type = Transient
[]


[Outputs]
  file_base = small_deform4
  exodus = true
  [./csv]
    type = CSV
    [../]
[]
