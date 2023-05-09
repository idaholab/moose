[Mesh]
  [gen_mesh]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0
    xmax = 10
    nx = 50
  []
  [left]
    type = ParsedSubdomainMeshGenerator
    input = 'gen_mesh'
    combinatorial_geometry = 'x < 0.5'
    block_id = '2'
  []
  [middle_boundary]
    type = SideSetsBetweenSubdomainsGenerator
    input = 'left'
    primary_block = '0'
    paired_block = '2'
    new_boundary = 'middle'
  []
[]

[Variables]
  [v]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    block = 2
  []
  [u]
    type = MooseVariableFVReal
  []
[]

[ICs]
  [v_ic]
    type = FunctionIC
    variable = v
    function = 'if (x > 2 & x < 3, 0.5, 0)'
  []
[]

[FVKernels]
  # Twice the kernel makes it not the Burgers equation, but shows the ordering
  [2_burger]
    type = FVBurgers1D
    variable = v
  []
  [1_burgers]
    type = FVBurgers1D
    variable = v
  []
  [time]
    type = FVTimeKernel
    variable = v
  []
  [time_u]
    type = FVTimeKernel
    variable = u
  []
[]

[FVBCs]
  [fv_burgers_right]
    type = FVBurgersOutflowBC
    variable = v
    boundary = 'middle'
  []
  [fv_burgers_left]
    type = FVBurgersOutflowBC
    variable = v
    boundary = 'left'
  []
[]

[FVInterfaceKernels]
  [diff_ik]
    type = FVOnlyAddDiffusionToOneSideOfInterface
    variable1 = 'v'
    variable2 = 'u'
    boundary = 'middle'
    coeff2 = '1'
    subdomain1 = '2'
    subdomain2 = '0'
  []
[]

[Executioner]
  type = Transient
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  petsc_options = '-snes_converged_reason'
  nl_abs_tol = 1e-7
  nl_rel_tol = 1e-8
  num_steps = 1
  dt = 0.05
  nl_forced_its = 1
  line_search = none
[]

[Debug]
  show_execution_order = 'LINEAR'
[]
