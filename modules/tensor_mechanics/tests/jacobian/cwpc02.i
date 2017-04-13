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

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  Cosserat_rotations = 'wc_x wc_y wc_z'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
  [./wc_x]
  [../]
  [./wc_y]
  [../]
[]

[Kernels]
  [./cx_elastic]
    type = CosseratStressDivergenceTensors
    variable = disp_x
    component = 0
  [../]
  [./cy_elastic]
    type = CosseratStressDivergenceTensors
    variable = disp_y
    component = 1
  [../]
  [./cz_elastic]
    type = CosseratStressDivergenceTensors
    variable = disp_z
    component = 2
  [../]
  [./x_couple]
    type = StressDivergenceTensors
    variable = wc_x
    displacements = 'wc_x wc_y wc_z'
    component = 0
    base_name = couple
  [../]
  [./y_couple]
    type = StressDivergenceTensors
    variable = wc_y
    displacements = 'wc_x wc_y wc_z'
    component = 1
    base_name = couple
  [../]
  [./x_moment]
    type = MomentBalancing
    variable = wc_x
    component = 0
  [../]
  [./y_moment]
    type = MomentBalancing
    variable = wc_y
    component = 1
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
  [./bottom_wc_x]
    type = PresetBC
    variable = wc_x
    boundary = back
    value = 0.0
  [../]
  [./bottom_wc_y]
    type = PresetBC
    variable = wc_y
    boundary = back
    value = 0.0
  [../]


  [./topx]
    type = FunctionPresetBC
    variable = disp_x
    boundary = front
    function = 32*t/3.333333333333E+00
  [../]
  [./topy]
    type = FunctionPresetBC
    variable = disp_y
    boundary = front
    function = 24*t/3.333333333333E+00
  [../]
  [./topz]
    type = FunctionPresetBC
    variable = disp_z
    boundary = front
    function = 10*t/8.108108108108E+00
  [../]
  [./top_wc_x]
    type = PresetBC
    variable = wc_x
    boundary = front
    value = 0.0
  [../]
  [./top_wc_y]
    type = PresetBC
    variable = wc_y
    boundary = front
    value = 0.0
  [../]
[]

[AuxVariables]
  [./wc_z]
  [../]
[]

[UserObjects]
  [./coh]
    type = TensorMechanicsHardeningConstant
    value = 20
  [../]
  [./tanphi]
    type = TensorMechanicsHardeningConstant
    value = 0.5
  [../]
  [./tanpsi]
    type = TensorMechanicsHardeningConstant
    value = 2.055555555556E-01
  [../]
  [./t_strength]
    type = TensorMechanicsHardeningConstant
    value = 100
  [../]
  [./c_strength]
    type = TensorMechanicsHardeningConstant
    value = 100
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeLayeredCosseratElasticityTensor
    young = 10.0
    poisson = 0.25
    layer_thickness = 10.0
    joint_normal_stiffness = 2.5
    joint_shear_stiffness = 2.0
  [../]
  [./strain]
    type = ComputeCosseratIncrementalSmallStrain
  [../]
  [./stress]
    type = ComputeCappedWeakPlaneCosseratStress
    cohesion = coh
    tan_friction_angle = tanphi
    tan_dilation_angle = tanpsi
    tensile_strength = t_strength
    compressive_strength = c_strength
    tip_smoother = 0
    smoothing_tol = 0
    yield_function_tol = 1E-5
  [../]
[]

[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -snes_type'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000 test'
  [../]
[]

[Executioner]
  solve_type = 'NEWTON'
  end_time = 1
  dt = 1
  type = Transient
[]
