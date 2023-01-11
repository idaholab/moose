[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  [../]
[]

[Problem]
  type = ReferenceResidualProblem
  reference_vector = 'ref'
  extra_tag_vectors = 'ref'
  group_variables = 'disp_x disp_y'
[]

[Variables]
  [./scalar_strain_zz]
    order = FIRST
    family = SCALAR
  [../]
[]

[AuxVariables]
  [./temp]
    order = FIRST
    family = LAGRANGE
  [../]
  [./saved_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./saved_y]
    order = FIRST
    family = LAGRANGE
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

[Modules/TensorMechanics/Master]
  [./all]
    strain = SMALL
    add_variables = true
    displacements = 'disp_x disp_y'
    generate_output = 'stress_xx stress_xy stress_yy stress_zz strain_xx strain_xy strain_yy strain_zz'
    planar_formulation = GENERALIZED_PLANE_STRAIN
    eigenstrain_names = eigenstrain
    scalar_out_of_plane_strain = scalar_strain_zz
    temperature = temp
    extra_vector_tags = 'ref'
  [../]
[]

[AuxKernels]
  [./tempfuncaux]
    type = FunctionAux
    variable = temp
    function = tempfunc
    use_displaced_mesh = false
  [../]
  [./saved_x]
    type = TagVectorAux
    variable = 'saved_x'
    vector_tag = 'ref'
    v = 'disp_x'
    execute_on = timestep_end
  [../]
  [./saved_y]
    type = TagVectorAux
    variable = 'saved_y'
    vector_tag = 'ref'
    execute_on = timestep_end
    v = 'disp_y'
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

[Materials]
  [./elastic_tensor]
    type = ComputeIsotropicElasticityTensor
    poissons_ratio = 0.3
    youngs_modulus = 1e6
  [../]
  [./thermal_strain]
    type = ComputeThermalExpansionEigenstrain
    temperature = temp
    thermal_expansion_coeff = 0.02
    stress_free_temperature = 0.5
    eigenstrain_name = eigenstrain
  [../]
  [./stress]
    type = ComputeLinearElasticStress
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient

  solve_type = PJFNK
  line_search = none

# controls for linear iterations
  l_max_its = 100
  l_tol = 1e-4

# controls for nonlinear iterations
  nl_max_its = 15
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-5

# time control
  start_time = 0.0
  dt = 1.0
  dtmin = 1.0
  end_time = 2.0
  num_steps = 5000
[]

[Outputs]
  exodus = true
[]
