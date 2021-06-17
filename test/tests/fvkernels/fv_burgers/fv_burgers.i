[Mesh]
  [./gen_mesh]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0
    xmax = 10
    nx = 50
  [../]
[]

[Variables]
  [./v]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  [../]
[]

[ICs]
  [./v_ic]
    type = FunctionIC
    variable = v
    function = 'if (x > 2 & x < 3, 0.5, 0)'
  [../]
[]

[FVKernels]
  [./burgers]
    type = FVBurgers1D
    variable = v
  [../]
  [./time]
    type = FVTimeKernel
    variable = v
  [../]
[]

[FVBCs]
  [./fv_burgers_outflow]
    type = FVBurgersOutflowBC
    variable = v
    boundary = 'left right'
  [../]
[]

[Executioner]
  type = Transient
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  petsc_options = '-snes_converged_reason'
  nl_abs_tol = 1e-7
  nl_rel_tol = 1e-8
  num_steps = 5
  dt = 0.05
[]

[Outputs]
  exodus = true
[]
