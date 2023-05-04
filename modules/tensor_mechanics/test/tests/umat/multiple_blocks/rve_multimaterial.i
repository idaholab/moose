[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = true
[]

[Mesh]
  [mesh_1]
    type = FileMeshGenerator
    file = rve.e
  []
[]

[Functions]
  [top_shear]
    type = ParsedFunction
    expression = t/0.05
  []
[]

[BCs]
  [fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = '1000'
    value = 0
  []
  [fix_y]
    type = DirichletBC
    variable = disp_y
    boundary = '1000'
    value = 0
  []
  [fix_z]
    type = DirichletBC
    variable = disp_z
    boundary = '1000'
    value = 0
  []
  [slip_x]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = '4000'
    function = top_shear
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    strain = FINITE
    generate_output = 'stress_yy'
    incremental = true
  []
[]

[Materials]
  [umat_1]
    type = AbaqusUMATStress
    # Young's modulus,  Poisson's Ratio, Yield, Hardening
    constant_properties = '1000 0.3'
    plugin = ../../../plugins/elastic_incremental
    num_state_vars = 3
    use_one_based_indexing = true
    block = '1'
  []
  [umat_2]
    type = AbaqusUMATStress
    # Young's modulus,  Poisson's Ratio
    constant_properties = '1e8 0.3'
    plugin = ../../../plugins/elastic_incremental
    num_state_vars = 3
    use_one_based_indexing = true
    block = '2'
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
  []
  [elastic_1]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1000
    poissons_ratio = 0.3
    block = '1'
  []
  [elastic_2]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e8
    poissons_ratio = 0.3
    block = '2'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  dt = 0.05
  solve_type = 'NEWTON'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'
  line_search = none

  nl_abs_tol = 1e-10
  dtmax = 10.0
  nl_rel_tol = 1e-10
  end_time = 1
  dtmin = 0.05
  num_steps = 2
[]

[Outputs]
  exodus = true
[]
