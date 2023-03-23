[Mesh]
  second_order = true
  inactive = 'mesh internal_boundary_bot internal_boundary_top'
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1'
    dy = '1 1 1'
    ix = '5'
    iy = '5 5 5'
    subdomain_id = '1
                    2
                    3'
  []
  [internal_boundary_bot]
    type = SideSetsBetweenSubdomainsGenerator
    input = mesh
    new_boundary = 'internal_bot'
    primary_block = 1
    paired_block = 2
  []
  [internal_boundary_top]
    type = SideSetsBetweenSubdomainsGenerator
    input = internal_boundary_bot
    new_boundary = 'internal_top'
    primary_block = 2
    paired_block = 3
  []
  [diverging_mesh]
    type = FileMeshGenerator
    file = 'expansion_quad.e'
  []
[]

[Modules]
  [IncompressibleNavierStokes]
    equation_type = steady-state

    # no slip BCs
    velocity_boundary = 'bottom right left'
    velocity_function = '0 1    0 0   0 0'
    pressure_boundary = 'top'
    pressure_function = '1'

    density_name = rho
    dynamic_viscosity_name = mu

    integrate_p_by_parts = false
    order = SECOND
  []
[]

[Materials]
  [const]
    type = GenericConstantMaterial
    block = '1 2 3'
    prop_names = 'rho mu'
    prop_values = '1  1'
  []
  [ADconst]
    type = ADGenericFunctorMaterial
    block = '1 2 3'
    prop_names = 'rho_ad'
    prop_values = '1'
  []
  [vel_functor]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'velocity'
    prop_values = 'vel_x vel_y 0'
  []
[]

[Preconditioning]
  [SMP_PJFNK]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-ksp_gmres_restart -pc_type -sub_pc_type -sub_pc_factor_levels'
  petsc_options_value = '300                bjacobi  ilu          4'
  line_search = none
  nl_rel_tol = 1e-12
  nl_max_its = 6
  l_tol = 1e-6
  l_max_its = 300
[]

[Postprocessors]
  [pdrop_total]
    type = PressureDrop
    pressure = p
    upstream_boundary = 'bottom'
    downstream_boundary = 'top'
    boundary = 'top bottom'
  []
  [pdrop_mid1]
    type = PressureDrop
    pressure = p
    upstream_boundary = 'bottom'
    downstream_boundary = 'internal_bot'
    boundary = 'bottom internal_bot'
  []
  [pdrop_mid2]
    type = PressureDrop
    pressure = p
    upstream_boundary = 'internal_bot'
    downstream_boundary = 'internal_top'
    boundary = 'internal_top internal_bot'
  []
  [pdrop_mid3]
    type = PressureDrop
    pressure = p
    upstream_boundary = 'internal_top'
    downstream_boundary = 'top'
    boundary = 'top internal_top'
  []
  [sum_drops]
    type = ParsedPostprocessor
    function = 'pdrop_mid1 + pdrop_mid2 + pdrop_mid3'
    pp_names = 'pdrop_mid1 pdrop_mid2 pdrop_mid3'
  []

  [p_upstream]
    type = SideAverageValue
    variable = p
    boundary = 'bottom'
  []
  [p_downstream]
    type = SideAverageValue
    variable = p
    boundary = 'top'
  []
[]

[Outputs]
  exodus = false
  csv = true
[]
