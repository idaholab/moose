# This input file tests Dirichlet pressure in/outflow boundary conditions for the incompressible NS equations.
[GlobalParams]
  gravity = '0 0 0'
[]

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

[Variables]
  [./vel_x]
    order = SECOND
    family = LAGRANGE
  [../]
  [./vel_y]
    order = SECOND
    family = LAGRANGE
  [../]
  [./p]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./mass]
    type = INSMass
    variable = p
    u = vel_x
    v = vel_y
    pressure = p
  [../]
  [./x_momentum_space]
    type = INSMomentumLaplaceForm
    variable = vel_x
    u = vel_x
    v = vel_y
    pressure = p
    component = 0
    integrate_p_by_parts = false
  [../]
  [./y_momentum_space]
    type = INSMomentumLaplaceForm
    variable = vel_y
    u = vel_x
    v = vel_y
    pressure = p
    component = 1
    integrate_p_by_parts = false
  [../]
[]

[BCs]
  [./x_no_slip]
    type = DirichletBC
    variable = vel_x
    boundary = 'top bottom'
    value = 0.0
  [../]
  [./y_no_slip]
    type = DirichletBC
    variable = vel_y
    boundary = 'left top bottom'
    value = 0.0
  [../]
  [./inlet_p]
    type = DirichletBC
    variable = p
    boundary = left
    value = 1.0
  [../]
  [./outlet_p]
    type = DirichletBC
    variable = p
    boundary = right
    value = 0.0
  [../]
[]

[Materials]
  [./const]
    type = GenericConstantMaterial
    block = 0
    prop_names = 'rho mu'
    prop_values = '1  1'
  [../]
[]

[Preconditioning]
  [./SMP_PJFNK]
    type = SMP
    full = true
    solve_type = PJFNK
  [../]
[]

[Executioner]
  type = Steady
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
