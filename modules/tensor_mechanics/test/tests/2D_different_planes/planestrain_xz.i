[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  file = square_xz_plane.e
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_z]
  [../]
[]

[AuxVariables]
  [./temp]
  [../]
  [./disp_y]
  [../]
[]

[Modules/TensorMechanics/Master]
  [./plane_strain]
    block = 1
    strain = SMALL
    out_of_plane_direction = y
    planar_formulation = PLANE_STRAIN
    eigenstrain_names = 'eigenstrain'
    generate_output = 'stress_xx stress_xz stress_yy stress_zz strain_xx strain_xz strain_yy strain_zz'
  [../]
[]

[AuxKernels]
  [./tempfuncaux]
    type = FunctionAux
    variable = temp
    function = tempfunc
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
    boundary = 3
    variable = disp_x
    value = 0.0
  [../]
  [./bottomy]
    type = DirichletBC
    boundary = 3
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
  [./react_y]
    type = MaterialTensorIntegral
    use_displaced_mesh = false
    rank_two_tensor = stress
    index_i = 1
    index_j = 1
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
  file_base = planestrain_xz_small_out
  exodus = true
[]
