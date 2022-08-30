[Mesh]
  [gen_mesh]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0
    xmax = 10
    nx = 50
  []
[]

[Variables]
  [v]
    family = MONOMIAL
    order = CONSTANT
    fv = true
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
[]

[FVBCs]
  [fv_burgers_right]
    type = FVBurgersOutflowBC
    variable = v
    boundary = 'right'
  []
  [fv_burgers_left]
    type = FVBurgersOutflowBC
    variable = v
    boundary = 'left'
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
[]

[Debug]
  show_execution_order = 'LINEAR'
[]
