# Testing the UMAT Interface - linear elastic model using the large strain formulation.

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [mesh_1]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = -0.5
    xmax = 0.5
    ymin = -0.5
    ymax = 0.5
    zmin = -0.5
    zmax = 0.5
    nx = 2
    ny = 2
    nz = 2
  []
  [block_1]
    type = SubdomainIDGenerator
    input = mesh_1
    subdomain_id = 1
  []
  [mesh_2]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = -2.0
    xmax = -1.0
    ymin = -2.0
    ymax = -1.0
    zmin = -2.0
    zmax = -1.0
    nx = 2
    ny = 2
    nz = 2
    boundary_name_prefix = 'second'
  []
  [block_2]
    type = SubdomainIDGenerator
    input = mesh_2
    subdomain_id = 2
  []
  [combined]
    type = CombinerGenerator
    inputs = 'block_1 block_2'
  []
[]

[Functions]
  [top_pull]
    type = ParsedFunction
    value = t/100
  []
  # Forced evolution of temperature
  [temperature_load]
    type = ParsedFunction
    value = '273'
  []
  # Factor to multiply the elasticity tensor in MOOSE
  [elasticity_prefactor]
    type = ParsedFunction
    value = '1'
  []
[]

[AuxVariables]
  [temperature]
  []
[]

[AuxKernels]
  [temperature_function]
    type = FunctionAux
    variable = temperature
    function = temperature_load
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    strain = FINITE
    generate_output = 'stress_yy'
  []
[]

[BCs]
  [y_pull_function]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top
    function = top_pull
  []
  [x_bot]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  []
  [y_bot]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []
  [z_bot]
    type = DirichletBC
    variable = disp_z
    boundary = front
    value = 0.0
  []
[]

[Materials]
  [umat_1]
    type = AbaqusUMATStress
    constant_properties = '1000 0.3'
    plugin = '../../../plugins/elastic_temperature'
    num_state_vars = 0
    temperature = temperature
    use_one_based_indexing = true
    block = '1'
  []
  # Linear strain hardening
  [umat_2]
    type = AbaqusUMATStress
    #  Young's modulus,  Poisson's Ratio, Yield, Hardening
    constant_properties = '1000 0.3 100 100'
    plugin = '../../../plugins/linear_strain_hardening'
    num_state_vars = 3
    use_one_based_indexing = true
    block = '2'
  []

  [elastic]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1000
    poissons_ratio = 0.3
    elasticity_tensor_prefactor = 'elasticity_prefactor'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'

  line_search = 'none'

  l_max_its = 100
  nl_max_its = 100
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-10
  l_tol = 1e-9
  start_time = 0.0
  num_steps = 30
  dt = 1.0
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Outputs]
  exodus = true
[]
