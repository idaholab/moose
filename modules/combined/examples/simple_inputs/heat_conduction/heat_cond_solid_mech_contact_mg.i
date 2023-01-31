[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [top_square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    ymin = 1.025
    ymax = 2.025
    boundary_name_prefix = top_square
  []
  [top_square_block]
    type = SubdomainIDGenerator
    input = top_square
    subdomain_id = 1
  []
  [bottom_square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    boundary_name_prefix = bottom_square
    boundary_id_offset = 4
  []
  [bottom_square_block]
    type = SubdomainIDGenerator
    input = bottom_square
    subdomain_id = 2
  []
  [two_blocks]
    type = MeshCollectionGenerator
    inputs = 'top_square_block bottom_square_block'
  []
  [block_rename]
    type = RenameBlockGenerator
    input = two_blocks
    old_block = '1 2'
    new_block = 'top_square bottom_square'
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

[Functions]
  [source]
    type = PiecewiseLinear
    x = '0 1 2'
    y = '0 1 0'
  []
[]

[Modules/TensorMechanics/Master]
  [top_square]
    block = top_square
    temperature = temperature
    add_variables = false
    strain = FINITE
  []
  [bottom_square]
    block = bottom_square
    temperature = temperature
    add_variables = false
    strain = FINITE
    eigenstrain_names = thermal_eigenstrain
  []
[]

[Kernels]
  [heat_conduction]
    type = HeatConduction
    variable = temperature
    block = 'bottom_square top_square'
  []
  [heat_source]
    type = HeatSource
    variable = temperature
    value = 1500
    block = bottom_square
  []
[]

[Materials]
  [heat_conductor]
    type = HeatConductionMaterial
    thermal_conductivity = 1
    block = 'top_square bottom_square'
  []
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = 'top_square bottom_square'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  []
  [thermal_expansion_strain]
    type = ComputeThermalExpansionEigenstrain
    stress_free_temperature = 200
    thermal_expansion_coeff = 1.0e-4
    temperature = temperature
    eigenstrain_name = thermal_eigenstrain
    block = 'bottom_square'
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
    block = 'top_square bottom_square'
  []
[]

[Contact]
  [mechanical]
    model = frictionless
    formulation = mortar
    primary = bottom_square_top
    secondary = top_square_bottom
    c_normal = 1e4
  []
[]

[BCs]
  [leftright_temp]
    type = DirichletBC
    variable = temperature
    boundary = 'bottom_square_left bottom_square_right'
    value = 200
  []
  [leftright_disp_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'bottom_square_left bottom_square_right'
    value = 0
  []
  [bottom_disp_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom_square_bottom
    value = 0
  []
  [bottom_disp_y_upper]
    type = DirichletBC
    variable = disp_y
    boundary = 'top_square_left top_square_top top_square_right'
    value = 0
  []
  [bottom_disp_x_upper]
    type = DirichletBC
    variable = disp_x
    boundary = 'top_square_left top_square_top top_square_right'
    value = 0
  []
  [temp_upper]
    type = DirichletBC
    variable = temperature
    boundary = top_square_top
    value = 200
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

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'
  line_search = none

  dt = 0.1
  dtmin = 0.01
  end_time = 1.0

  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-9

  l_max_its = 100
  nl_max_its = 25
[]

[Outputs]
  exodus = true
[]

[Postprocessors]
  [peak_temp]
    type = NodalExtremeValue
    variable = temperature
  []
[]
