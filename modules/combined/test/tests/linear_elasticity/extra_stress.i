[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 128
  ny = 1
  xmax = 3.2
  ymax = 0.025
  elem_type = QUAD4
[]

[Physics/SolidMechanics/QuasiStatic/All]
  add_variables = true
  generate_output = 'stress_xx stress_xy stress_yy stress_zz strain_xx strain_xy strain_yy'
[]

[AuxVariables]
  [./c]
  [../]
[]

[ICs]
  [./c_IC]
    type = BoundingBoxIC
    variable = c
    x1 = -1
    y1 = -1
    x2 = 1.6
    y2 = 1
    inside = 0
    outside = 1
    block = 0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    C_ijkl = '104 74 74 104 74 104 47.65 47.65 47.65'
    fill_method = symmetric9
    base_name = matrix
  [../]
  [./stress]
    type = ComputeLinearElasticStress
    block = 0
    base_name = matrix
  [../]
  [./strain]
    type = ComputeSmallStrain
    block = 0
    base_name = matrix
  [../]
  [./elasticity_tensor_ppt]
    type = ComputeElasticityTensor
    block = 0
    C_ijkl = '0.104 0.074 0.074 0.104 0.074 0.104 0.04765 0.04765 0.04765'
    fill_method = symmetric9
    base_name = ppt
  [../]
  [./stress_ppt]
    type = ComputeLinearElasticStress
    block = 0
    base_name = ppt
  [../]
  [./strain_ppt]
    type = ComputeSmallStrain
    block = 0
    base_name = ppt
  [../]
  [./const_stress]
    type = ComputeExtraStressConstant
    block = 0
    base_name = ppt
    extra_stress_tensor = '-0.288 -0.373 -0.2747 0 0 0'
  [../]
  [./global_stress]
    type = TwoPhaseStressMaterial
    base_A = matrix
    base_B = ppt
  [../]
  [./switching]
    type = SwitchingFunctionMaterial
    eta = c
  [../]
[]

[BCs]
  active = 'left_x right_x bottom_y top_y'
  [./bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  [../]
  [./left_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  [../]
  [./right_x]
    type = DirichletBC
    variable = disp_x
    boundary = right
    value = 0
  [../]
  [./top_y]
    type = DirichletBC
    variable = disp_y
    boundary = top
    value = 0
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
