[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  xmin = -0.5
  ymin = -0.5
  xmax = 0.5
  ymax = 0.5
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

[Kernels]
  [./TensorMechanics]
    disp_x = disp_x
    disp_y = disp_y
  [../]
[]

[AuxVariables]
  [./c]
  [../]
  [./C11_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./s11_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
 [./matl_s11]
    type = RankTwoAux
    variable = s11_aux
    rank_two_tensor = stress
    index_i = 0
    index_j = 0
  [../]
  [./matl_C11]
    type = RankFourAux
    variable = C11_aux
    rank_four_tensor = elasticity_tensor
    index_l = 0
    index_j = 0
    index_k = 0
    index_i = 0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeConcentrationDependentElasticityTensor
    block = 0
    c = c
    C1_ijkl = '6 6'
    C0_ijkl = '1 1'
    fill_method1 = symmetric_isotropic
    fill_method0 = symmetric_isotropic
  [../]
  [./stress]
    type = ComputeLinearElasticStress
    block = 0
  [../]
  [./strain]
    type = ComputeSmallStrain
    block = 0
    disp_x = disp_x
    disp_y = disp_y
  [../]
[]

[BCs]
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
    value = 0.5
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
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 31'
  l_max_its = 20
  nl_max_its = 10
  l_tol = 1.0e-4
  nl_rel_tol = 1.0e-6
  nl_abs_tol = 1.0e-8
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]

[ICs]
  [./c_IC]
    int_width = 0.2
    x1 = 0
    y1 = 0
    radius = 0.25
    outvalue = 0
    variable = c
    invalue = 1
    type = SmoothCircleIC
  [../]
[]
