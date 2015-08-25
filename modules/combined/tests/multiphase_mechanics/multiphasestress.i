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
  [./eta1]
    [./InitialCondition]
      type = FunctionIC
      function = 'x/2'
    [../]
  [../]
  [./eta2]
    [./InitialCondition]
      type = FunctionIC
      function = 'y/2'
    [../]
  [../]
  [./eta3]
    [./InitialCondition]
      type = FunctionIC
      function = '(2^0.5-(y-1)^2=(y-1)^2)/2'
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
    C_ijkl = '1e6 0 0 1e6 0 1.1e6 .5e6 .5e6 .5e6'
    applied_strain_vector = '0.1 0.05 0 0 0 0.01'
  [../]
  [./Anisotropic_C]
    type = LinearElasticMaterial
    base_name = C
    block = 0
    disp_x = disp_x
    disp_y = disp_y
    fill_method = symmetric9
    C_ijkl = '1.1e6 1e5 0 1e6 0 1e6 .5e6 .2e6 .5e6'
    applied_strain_vector = '0.1 0.05 0 0 0 0.01'
  [../]

  [./switching_A]
    type = SwitchingFunctionMaterial
    block = 0
    function_name = h1
    eta = eta1
  [../]
  [./switching_B]
    type = SwitchingFunctionMaterial
    block = 0
    function_name = h2
    eta = eta2
  [../]
  [./switching_C]
    type = SwitchingFunctionMaterial
    block = 0
    function_name = h3
    eta = eta3
  [../]

  [./combined]
    type = MultiPhaseStressMaterial
    block = 0
    phase_base = 'A  B  C'
    h          = 'h1 h2 h3'
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
  execute_on = 'timestep_end'
  exodus = true
  print_linear_residuals = true
  print_perf_log = true
[]
