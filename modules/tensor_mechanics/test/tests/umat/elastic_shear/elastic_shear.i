[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Functions]
  [tdisp]
    type = ParsedFunction
    expression = '0.025 * t'
  []
[]

[Mesh]
  [msh]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    zmin = 0
    zmax = 1
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    strain = FINITE
    generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_xz stress_yz'
  []
[]

[BCs]
  [bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = bottom
    value = 0
  []
  [bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [bottom_z]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0
  []
  [top_y]
    type = DirichletBC
    variable = disp_y
    boundary = top
    value = 0
  []
  [top_z]
    type = DirichletBC
    variable = disp_z
    boundary = top
    value = 0
  []
  [tdisp]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = top
    function = tdisp
  []
[]

[Materials]
  [umat]
    type = AbaqusUMATStress
    # c10=G/2   D=2/K
    constant_properties = '5 0.025'
    plugin = '../../../plugins/neo_hooke'
    num_state_vars = 0
    use_one_based_indexing = true
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
  end_time = 20

  dt = 10.0
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Outputs]
  exodus = true
  interval = 1
[]
