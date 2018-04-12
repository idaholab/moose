[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  file = square_yz_plane.e
[]

[Variables]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
  [./scalar_strain_xx]
    order = FIRST
    family = SCALAR
  [../]
[]

[AuxVariables]
  [./temp]
  [../]
  [./disp_x]
  [../]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_yz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Modules/TensorMechanics/Master]
  [./generalized_plane_strain]
    block = 1
    strain = SMALL
    scalar_out_of_plane_strain = scalar_strain_xx
    out_of_plane_direction = x
    planar_formulation = GENERALIZED_PLANE_STRAIN
    eigenstrain_names = 'eigenstrain'
  [../]
[]

[AuxKernels]
  [./tempfuncaux]
    type = FunctionAux
    variable = temp
    function = tempfunc
  [../]
  [./stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_j = 0
    index_i = 0
  [../]
  [./stress_yz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yz
    index_j = 2
    index_i = 1
  [../]
  [./stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_j = 1
    index_i = 1
  [../]
  [./stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_j = 2
    index_i = 2
  [../]

  [./strain_xx]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_xx
    index_j = 0
    index_i = 0
  [../]
  [./strain_yz]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_yz
    index_j = 2
    index_i = 1
  [../]
  [./strain_yy]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_yy
    index_j = 1
    index_i = 1
  [../]
  [./strain_zz]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_zz
    index_j = 2
    index_i = 2
  [../]
[]

[Functions]
  [./tempfunc]
    type = ParsedFunction
    value = '(1-y)*t'
  [../]
[]

[BCs]
  [./bottomx]
    type = PresetBC
    boundary = 4
    variable = disp_y
    value = 0.0
  [../]
  [./bottomy]
    type = PresetBC
    boundary = 4
    variable = disp_z
    value = 0.0
  [../]
[]

[Materials]
  [./elastic_stress]
    type = ComputeLinearElasticStress
    block = 1
  [../]
  [./thermal_strain]
    type = ComputeThermalExpansionEigenstrain
    temperature = temp
    thermal_expansion_coeff = 0.02
    stress_free_temperature = 0.5
    eigenstrain_name = eigenstrain
  [../]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = 1
    poissons_ratio = 0.3
    youngs_modulus = 1e6
  [../]
[]

[Postprocessors]
  [./react_x]
    type = MaterialTensorIntegral
    use_displaced_mesh = false
    rank_two_tensor = stress
    index_i = 0
    index_j = 0
  [../]
[]

[Executioner]
  type = Transient

  solve_type = PJFNK
  line_search = none

# controls for linear iterations
  l_max_its = 100
  l_tol = 1e-10

# controls for nonlinear iterations
  nl_max_its = 10
  nl_rel_tol = 1e-12

# time control
  start_time = 0.0
  dt = 1.0
  dtmin = 1.0
  end_time = 2.0
[]

[Outputs]
  file_base = gps_yz_small_out
  [./exodus]
    type = Exodus
    output_dimension = 3
  [../]
[]
