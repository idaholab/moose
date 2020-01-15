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

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
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
  [../]
[]

[Materials]
  [./elasticity_tensor_A]
    type = ComputeElasticityTensor
    base_name = A
    fill_method = symmetric9
    C_ijkl = '1e6 1e5 1e5 1e6 0 1e6 .4e6 .2e6 .5e6'
  [../]
  [./strain_A]
    type = ComputeSmallStrain
    base_name = A
    eigenstrain_names = eigenstrain
  [../]
  [./stress_A]
    type = ComputeLinearElasticStress
    base_name = A
  [../]
  [./eigenstrain_A]
    type = ComputeEigenstrain
    base_name = A
    eigen_base = '0.1 0.05 0 0 0 0.01'
    prefactor = -1
    eigenstrain_name = eigenstrain
  [../]

  [./elasticity_tensor_B]
    type = ComputeElasticityTensor
    base_name = B
    fill_method = symmetric9
    C_ijkl = '1e6 0 0 1e6 0 1e6 .5e6 .5e6 .5e6'
  [../]
  [./strain_B]
    type = ComputeSmallStrain
    base_name = B
    eigenstrain_names = 'B_eigenstrain'
  [../]
  [./stress_B]
    type = ComputeLinearElasticStress
    base_name = B
  [../]
  [./eigenstrain_B]
    type = ComputeEigenstrain
    base_name = B
    eigen_base = '0.1 0.05 0 0 0 0.01'
    prefactor = -1
    eigenstrain_name = 'B_eigenstrain'
  [../]

  [./switching]
    type = SwitchingFunctionMaterial
    eta = eta
  [../]
  [./combined]
    type = TwoPhaseStressMaterial
    base_A = A
    base_B = B
  [../]
[]

[BCs]
  [./bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom'
    value = 0
  [../]
  [./left_x]
    type = DirichletBC
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
[]
