[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = true
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    nx = 5
    ny = 5
    dim = 2
  []
  [block1]
    type = SubdomainBoundingBoxGenerator
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '0.5 1 0'
    input = gen
  []
  [block2]
    type = SubdomainBoundingBoxGenerator
    block_id = 2
    bottom_left = '0.5 0 0'
    top_right = '1 1 0'
    input = block1
  []
  [breakmesh]
    input = block2
    type = BreakMeshByBlockGenerator
    block_pairs = '1 2'
    split_interface = true
    add_interface_on_two_sides = true
  []
[]

[Variables]
  [temperature]
  []
  [disp_x]
  []
  [disp_y]
  []
[]

[Kernels]
  [thermal_cond]
    type = HeatConduction
    variable = temperature
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  generate_output = 'stress_xx stress_yy strain_xx strain_yy'
  add_variables = true
  strain = FINITE
  incremental = true
  [block1]
    block = 1
  []
  [block2]
    block = 2
  []
[]

[ThermalContact]
  [thermal_contact]
    type = GapHeatTransfer
    variable = temperature
    primary = Block1_Block2
    secondary = Block2_Block1
    emissivity_primary = 0
    emissivity_secondary = 0
    quadrature = true
    gap_conductivity = 1
  []
[]

[Contact]
  [mechanical]
    primary = Block1_Block2
    secondary = Block2_Block1
    penalty = 1000
    model = coulomb
    friction_coefficient = 0.5
    formulation = tangential_penalty
    tangential_tolerance = 0.1
  []
[]

[BCs]
  [left_temp]
    type = DirichletBC
    value = 100
    variable = temperature
    boundary = left
  []
  [right_temp]
    type = DirichletBC
    value = 0
    variable = temperature
    boundary = right
  []
  [left_disp_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = left
    function = 0
  []
  [left_disp_y]
    type = DirichletBC
    variable = disp_y
    boundary = left
    value = 0.0
  []
  [right_disp_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = right
    function = '-t'
  []
  [right_disp_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = right
    function = '0'
  []
[]

[Materials]
  [thermal_cond]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = 1
  []
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    poissons_ratio = 0.3
    youngs_modulus = 100
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Dampers]
  [contact_slip]
    type = ContactSlipDamper
    secondary = Block1_Block2
    primary = Block2_Block1
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'

  line_search = none

  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-9

  l_tol = 1e-4
  l_max_its = 50

  nl_max_its = 20

  start_time = 0.0

  num_steps = 2

  dtmin = 1e-8
  dt = 1e-2
  automatic_scaling = true
[]

[Outputs]
  print_linear_residuals = false
  time_step_interval = 1
  csv = false
  perf_graph = false
  exodus = true
[]
