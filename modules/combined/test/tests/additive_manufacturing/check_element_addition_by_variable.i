[Problem]
  kernel_coverage_check = false
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = 0
    xmax = 2.0
    ymin = 0
    ymax = 2.0
    zmin = 0
    zmax = 2.0
    nx = 10
    ny = 10
    nz = 10
  []
  [left_domain]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0 0 0'
    top_right = '2 2 1'
    block_id = 1
  []
  [right_domain]
    input = left_domain
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0 0 1'
    top_right = '2 2 2'
    block_id = 2
  []
  [sidesets]
    input = right_domain
    type = SideSetsAroundSubdomainGenerator
    normal = '0 0 1'
    block = 1
    new_boundary = 'moving_interface'
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  block = '1 2'
[]

[Modules/TensorMechanics/Master]
  [all]
    # strain = FINITE
    add_variables = true
    generate_output = 'stress_zz strain_zz'
    block = '1 2'
    use_automatic_differentiation = true
  []
[]

[Materials]
  [elasticity]
    type = ADComputeVariableIsotropicElasticityTensor
    poissons_ratio = 0.3
    youngs_modulus = 1e3
    block = '1 2'
  []
  [stress]
    type = ADComputeLinearElasticStress
    block = '1 2'
  []
[]

[Functions]
  [front_pull]
    type = PiecewiseLinear
    x = '0 1'
    y = '0 1'
    scale_factor = 0.5
  []
[]

[BCs]
  [disp_front_pull]
    type = ADFunctionDirichletBC
    variable = disp_z
    boundary = front
    function = front_pull
  []
  [uz_back_fix]
    type = ADDirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  []
  [u_yz_fix]
    type = ADDirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  []
  [u_xz_fix]
    type = ADDirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
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

  automatic_scaling = true

  solve_type = 'NEWTON'

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  line_search = 'none'

  l_max_its = 10
  nl_max_its = 20
  nl_rel_tol = 1e-4

  start_time = 0.0
  end_time = 1.0
  dt = 1e-1
  dtmin = 1e-4
[]

[UserObjects]
  [activated_elem_uo]
    type = ActivateElementsCoupled
    execute_on = timestep_begin
    coupled_var = strain_zz
    activate_value = 0.05
    active_subdomain_id = 1
    expand_boundary_name = 'moving_interface'
  []
[]

[Outputs]
  exodus = true
[]
