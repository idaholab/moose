[Mesh]
  [file_mesh]
    type = FileMeshGenerator
    file = circle.e
  []
  [cnode]
    type = ExtraNodesetGenerator
    coord = '1000.0 0.0'
    new_boundary = 10
    input = file_mesh
  []
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[AuxVariables]
  [./T]
  [../]
  [./stress_rr]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_tt]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[ICs]
  [./T_IC]
    type = FunctionIC
    variable = T
    function = '1000-0.7*sqrt(x^2+y^2)'
  [../]
[]

[Kernels]
  [./TensorMechanics]
    displacements = 'disp_x disp_y'
  [../]
[]

[AuxKernels]
  [./stress_rr]
    type = CylindricalRankTwoAux
    variable = stress_rr
    rank_two_tensor = stress
    index_j = 0
    index_i = 0
    center_point = '0 0 0'
  [../]
  [./stress_tt]
    type = CylindricalRankTwoAux
    variable = stress_tt
    rank_two_tensor = stress
    index_j = 1
    index_i = 1
    center_point = '0 0 0'
  [../]
[]

[BCs]
  [./outer_x]
    type = DirichletBC
    variable = disp_x
    boundary = 2
    value = 0
  [../]
  [./outer_y]
    type = DirichletBC
    variable = disp_y
    boundary = '2 10'
    value = 0
  [../]
[]

[Materials]
  [./iso_C]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '2.15e5 0.74e5'
    block = 1
  [../]
  [./strain]
    type = ComputeSmallStrain
    displacements = 'disp_x disp_y'
    block = 1
    eigenstrain_names = eigenstrain
  [../]
  [./stress]
    type = ComputeLinearElasticStress
    block = 1
  [../]
  [./thermal_strain]
    type= ComputeThermalExpansionEigenstrain
    thermal_expansion_coeff = 1e-6
    temperature = T
    stress_free_temperature = 273
    block = 1
    eigenstrain_name = eigenstrain
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 101'
  l_max_its = 30
  nl_max_its = 10
  nl_abs_tol = 1e-9
  nl_rel_tol = 1e-14
  l_tol = 1e-4
[]

[Outputs]
  exodus = true
  perf_graph = true
[]
