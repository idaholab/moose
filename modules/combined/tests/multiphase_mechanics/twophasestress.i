[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  xmin = 0
  xmax = 2
  ymin = 0
  ymax = 2
  elem_type = QUAD4
[]

[Variables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./eta]
    [./InitialCondition]
      type = FunctionIC
      function = 'x/2'
    [../]
  [../]
  [./e11_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./matl_e11]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 0
    index_j = 0
    variable = e11_aux
  [../]
[]

[Kernels]
  [./TensorMechanics]
    disp_x = disp_x
    disp_y = disp_y
  [../]
[]

[Materials]
  # active = 'Anisotropic'
  active = 'Anisotropic_A Anisotropic_B switching combined'

  [./Anisotropic]
    type = LinearElasticMaterial
    block = 0
    disp_x = disp_x
    disp_y = disp_y
    fill_method = symmetric9
    C_ijkl = '1e6 1e5 1e5 1e6 0 1e6 .4e6 .2e6 .5e6'
    applied_strain_vector = '0.1 0.05 0 0 0 0.01'
  [../]
  [./Anisotropic_A]
    type = LinearElasticMaterial
    base_name = A
    block = 0
    disp_x = disp_x
    disp_y = disp_y
    fill_method = symmetric9
    C_ijkl = '1e6 1e5 1e5 1e6 0 1e6 .4e6 .2e6 .5e6'
    applied_strain_vector = '0.1 0.05 0 0 0 0.01'
  [../]
  [./Anisotropic_B]
    type = LinearElasticMaterial
    base_name = B
    block = 0
    disp_x = disp_x
    disp_y = disp_y
    fill_method = symmetric9
    C_ijkl = '1e6 0 0 1e6 0 1e6 .5e6 .5e6 .5e6'
    applied_strain_vector = '0.1 0.05 0 0 0 0.01'
  [../]

  [./switching]
    type = SwitchingFunctionMaterial
    block = 0
    eta = eta
  [../]
  [./combined]
    type = TwoPhaseStressMaterial
    block = 0
    base_A = A
    base_B = A
    outputs = exodus
  [../]
[]

[BCs]
  [./bottom_y]
    type = PresetBC
    variable = disp_y
    boundary = 'bottom'
    value = 0
  [../]
  [./left_x]
    type = PresetBC
    variable = disp_x
    boundary = 'left'
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
  output_initial = false
  exodus = true
  print_linear_residuals = true
  print_perf_log = true
[]
