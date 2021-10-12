[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 3.0
  ymin = 0
  ymax = 1.0
  nx = 30
  ny = 10
  elem_type = QUAD9
[]

[Modules]
  [IncompressibleNavierStokes]
    equation_type = steady-state

    velocity_boundary = 'bottom top  left'
    velocity_function = '0 0    0 0  NA 0'
    pressure_boundary = 'left right'
    pressure_function = '1    0'

    density_name = rho
    dynamic_viscosity_name = mu

    integrate_p_by_parts = false
    order = SECOND
  []
[]

[Materials]
  [const]
    type = GenericConstantMaterial
    block = 0
    prop_names = 'rho mu'
    prop_values = '1  1'
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

[Outputs]
  file_base = open_bc_out_pressure_BC
  exodus = true
[]
