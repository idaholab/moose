# Testing the UMAT Interface - linear elastic model using the large strain formulation.

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = true
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = -0.5
    xmax = 0.5
    ymin = -0.5
    ymax = 0.5
    zmin = -0.5
    zmax = 0.5
    nx = 1
    ny = 1
    nz = 1
  []
[]

[Functions]
  [top_pull]
    type = ParsedFunction
    expression = 1.0e-5*t
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    strain = FINITE
    generate_output = 'stress_xy stress_yz stress_xz strain_xy strain_yz strain_xz'
  []
[]

[BCs]
  [pull_function]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = top
    function = top_pull
  []
  [x_bot]
    type = DirichletBC
    variable = disp_x
    boundary = bottom
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
    boundary = bottom
    value = 0.0
  []
[]

[Materials]
  # This input file is used to compare the MOOSE and UMAT models, activating
  # specific ones with cli variable_names.

  # 1. Active for umat calculation
  [umat]
    type = AbaqusUMATStress
    constant_properties = ' '
    plugin = '../../../plugins/elastic_incremental_anisotropic'
    num_state_vars = 0
    use_one_based_indexing = true
  []

  # 2. Active for reference MOOSE computations
  [elastic]
    type = ComputeElasticityTensor
    fill_method = orthotropic
    C_ijkl = '1.0e5 1.0e5 1.0e5 1.0e4 2.0e4 3.0e4 0.0 0.0 0.0 0.0 0.0 0.0'
    # skip_check = true
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
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
  nl_rel_tol = 1e-11
  nl_abs_tol = 1e-11
  l_tol = 1e-9
  start_time = 0.0
  end_time = 10.0
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
