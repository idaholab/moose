[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y'
  block = 0
[]

[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  [../]
[]

[AuxVariables]
  [./temp]
  [../]
  [./saved_x]
  [../]
  [./saved_y]
  [../]
[]

[AuxKernels]
  [./tempfuncaux]
    type = FunctionAux
    variable = temp
    function = tempfunc
    use_displaced_mesh = false
  [../]
[]

[Functions]
  [./tempfunc]
    type = ParsedFunction
    expression = '(1-x)*t'
  [../]
[]

[BCs]
  [./bottomx]
    type = DirichletBC
    boundary = 0
    variable = disp_x
    value = 0.0
  [../]
  [./bottomy]
    type = DirichletBC
    boundary = 0
    variable = disp_y
    value = 0.0
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = SMALL
    add_variables = true
    temperature = temp
    generate_output = 'stress_xx stress_xy stress_yy stress_zz strain_xx strain_xy strain_yy strain_zz'
    planar_formulation = PLANE_STRAIN
    eigenstrain_names = eigenstrain
    save_in = 'saved_x saved_y'
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]
  [./elastic_stress]
    type = ComputeLinearElasticStress
  [../]
  [./thermal_strain]
    type = ComputeThermalExpansionEigenstrain
    thermal_expansion_coeff = 0.02
    temperature = temp
    stress_free_temperature = 0.5
    eigenstrain_name = eigenstrain
  [../]
[]

[Postprocessors]
  [./react_z]
    type = MaterialTensorIntegral
    rank_two_tensor = stress
    index_i = 2
    index_j = 2
  [../]
[]

[Executioner]
  type = Transient

  solve_type = PJFNK
  line_search = none

  # controls for linear iterations
  l_max_its = 100
  l_tol = 1e-8

  # controls for nonlinear iterations
  nl_max_its = 15
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12

  # time control
  start_time = 0.0
  dt = 1.0
  dtmin = 1.0
  end_time = 2.0
[]

[Outputs]
  exodus = true
[]
