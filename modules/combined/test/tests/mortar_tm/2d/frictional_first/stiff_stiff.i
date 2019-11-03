E_block = 1e9
E_plank = 1e9
elem = QUAD4
order = FIRST
name = 'stiff_stiff'

[Mesh]
  patch_size = 80
  patch_update_strategy = auto
  [./plank]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -0.3
    xmax = 0.3
    ymin = -10
    ymax = 10
    nx = 2
    ny = 67
    elem_type = ${elem}
  [../]
  [./plank_sidesets]
    type = RenameBoundaryGenerator
    input = plank
    old_boundary_id = '0 1 2 3'
    new_boundary_name = 'plank_bottom plank_right plank_top plank_left'
  [../]
  [./plank_id]
    type = SubdomainIDGenerator
    input = plank_sidesets
    subdomain_id = 1
  [../]

  [./block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0.31
    xmax = 0.91
    ymin = 9.2
    ymax = 10.0
    nx = 3
    ny = 4
    elem_type = ${elem}
  [../]
  [./block_id]
    type = SubdomainIDGenerator
    input = block
    subdomain_id = 2
  [../]

  [./combined]
    type = MeshCollectionGenerator
    inputs = 'plank_id block_id'
  [../]
  [./block_rename]
    type = RenameBlockGenerator
    input = combined
    old_block_id = '1 2'
    new_block_name = 'plank block'
  [../]
  [./block_sidesets]
    type = SideSetsFromPointsGenerator
    input = block_rename
    points = '0.6  9.2  0
              0.91 9.5  0
              0.6  10.0 0
              0.31 9.5  0'
    new_boundary = 'block_bottom block_right block_top block_left'
  [../]
  [slave]
    input = block_sidesets
    type = LowerDBlockFromSidesetGenerator
    new_block_name = frictional_slave_subdomain
    sidesets = 'block_left'
  []
  [master]
    input = slave
    type = LowerDBlockFromSidesetGenerator
    new_block_name = frictional_master_subdomain
    sidesets = 'plank_right'
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./disp_x]
    order = ${order}
    block = 'plank block'
    scaling = ${fparse 2.0 / (E_plank + E_block)}
  [../]
  [./disp_y]
    order = ${order}
    block = 'plank block'
    scaling = ${fparse 2.0 / (E_plank + E_block)}
  [../]
  [./frictional_normal_lm]
    type = MooseVariable
    block = frictional_slave_subdomain
    scaling = 1e-7
  [../]
  [./frictional_tangential_lm]
    type = MooseVariableConstMonomial
    block = frictional_slave_subdomain
    scaling = 1e-6
  [../]
[]

[Modules/TensorMechanics/Master]
  [./action]
    decomposition_method = EigenSolution
    strain = FINITE
    generate_output = 'stress_xx stress_yy stress_zz vonmises_stress hydrostatic_stress strain_xx strain_yy strain_zz'
    block = 'plank block'
  [../]
[]

[Constraints]
  [./frictional_normal_lm]
    type = NormalNodalLMMechanicalContact
    disp_y = disp_y
    master = plank_right
    master_variable = disp_x
    slave = block_left
    variable = frictional_normal_lm
    c = 1e8
  [../]
  [./frictional_tangential_lm]
    type = TangentialMortarLMMechanicalContact
    contact_pressure = frictional_normal_lm
    friction_coefficient = 0.1
    master_boundary = plank_right
    master_subdomain = frictional_master_subdomain
    slave_boundary = block_left
    slave_disp_y = disp_y
    slave_subdomain = frictional_slave_subdomain
    slave_variable = disp_x
    use_displaced_mesh = true
    variable = frictional_tangential_lm
    c = 1e6
  [../]
  [./frictional_normal_constraint_0]
    type = NormalMortarMechanicalContact
    component = x
    compute_lm_residuals = false
    master_boundary = plank_right
    master_subdomain = frictional_master_subdomain
    slave_boundary = block_left
    slave_subdomain = frictional_slave_subdomain
    slave_variable = disp_x
    use_displaced_mesh = true
    variable = frictional_normal_lm
  [../]
  [./frictional_normal_constraint_1]
    type = NormalMortarMechanicalContact
    component = y
    compute_lm_residuals = false
    master_boundary = plank_right
    master_subdomain = frictional_master_subdomain
    slave_boundary = block_left
    slave_subdomain = frictional_slave_subdomain
    slave_variable = disp_y
    use_displaced_mesh = true
    variable = frictional_normal_lm
  [../]
  [./frictional_tangential_constraint_0]
    type = TangentialMortarMechanicalContact
    component = x
    compute_lm_residuals = false
    master_boundary = plank_right
    master_subdomain = frictional_master_subdomain
    slave_boundary = block_left
    slave_subdomain = frictional_slave_subdomain
    slave_variable = disp_x
    use_displaced_mesh = true
    variable = frictional_tangential_lm
  [../]
  [./frictional_tangential_constraint_1]
    type = TangentialMortarMechanicalContact
    component = y
    compute_lm_residuals = false
    master_boundary = plank_right
    master_subdomain = frictional_master_subdomain
    slave_boundary = block_left
    slave_subdomain = frictional_slave_subdomain
    slave_variable = disp_y
    use_displaced_mesh = true
    variable = frictional_tangential_lm
  [../]
[]

[BCs]
  [./left_x]
    type = DirichletBC
    variable = disp_x
    boundary = plank_left
    value = 0.0
  [../]
  [./left_y]
    type = DirichletBC
    variable = disp_y
    boundary = plank_bottom
    value = 0.0
  [../]
  [./right_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = block_right
    function = '-0.04*sin(4*t)+0.02'
  [../]
  [./right_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = block_right
    function = '-t'
  [../]
[]

[Materials]
  [./plank]
    type = ComputeIsotropicElasticityTensor
    block = 'plank'
    poissons_ratio = 0.3
    youngs_modulus = ${E_plank}
  [../]
  [./block]
    type = ComputeIsotropicElasticityTensor
    block = 'block'
    poissons_ratio = 0.3
    youngs_modulus = ${E_block}
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
    block = 'plank block'
  [../]
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
  petsc_options = '-snes_converged_reason -ksp_converged_reason'
  petsc_options_iname = '-pc_type -mat_mffd_err -pc_factor_shift_type -pc_factor_shift_amount -snes_max_it'
  petsc_options_value = 'lu       1e-5          NONZERO               1e-15                   20'
  end_time = 15
  dt = 0.1
  dtmin = 0.1
  timestep_tolerance = 1e-6
  l_max_its = 30
  nl_abs_tol = 1e-12
  line_search = none
[]

[Postprocessors]
  [./nl_its]
    type = NumNonlinearIterations
  [../]
  [./total_nl_its]
    type = CumulativeValuePostprocessor
    postprocessor = nl_its
  [../]
  [./l_its]
    type = NumLinearIterations
  [../]
  [./total_l_its]
    type = CumulativeValuePostprocessor
    postprocessor = l_its
  [../]
  [./contact]
    type = ContactDOFSetSize
    variable = frictional_normal_lm
    subdomain = frictional_slave_subdomain
    execute_on = 'nonlinear timestep_end'
    outputs = 'console out'
  [../]
  [tangential_contact]
    type = GreaterThanLessThanPostprocessor
    variable = frictional_tangential_lm
    execute_on = 'nonlinear timestep_end'
    outputs = 'console out'
    value = 1
  []
  [./avg_hydro]
    type = ElementAverageValue
    variable = hydrostatic_stress
    block = 'block'
  [../]
  [./max_hydro]
    type = ElementExtremeValue
    variable = hydrostatic_stress
    block = 'block'
  [../]
  [./min_hydro]
    type = ElementExtremeValue
    variable = hydrostatic_stress
    block = 'block'
    value_type = min
  [../]
  [./avg_vonmises]
    type = ElementAverageValue
    variable = vonmises_stress
    block = 'block'
  [../]
  [./max_vonmises]
    type = ElementExtremeValue
    variable = vonmises_stress
    block = 'block'
  [../]
  [./min_vonmises]
    type = ElementExtremeValue
    variable = vonmises_stress
    block = 'block'
    value_type = min
  [../]
[]

[Outputs]
  file_base = ${name}
  [./exodus_out]
    type = Exodus
    # execute_on = 'nonlinear'
  [../]
  [./out]
    type = CSV
    file_base = '${name}_out'
    execute_on = 'nonlinear timestep_end'
    show = 'contact tangential_contact'
  [../]
  checkpoint = true
[]

[Debug]
  show_var_residual_norms = true
[]
