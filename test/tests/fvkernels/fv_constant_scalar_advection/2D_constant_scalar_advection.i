[Mesh]
  [gen_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 2
    ymin = 0
    ymax = 4
    nx = 10
    ny = 20
  []
[]

[Variables]
  [v]
    type = MooseVariableFVReal
    two_term_boundary_expansion = false
  []
[]

[ICs]
  [v_ic]
    type = FunctionIC
    variable = v
    function = 'r2 := (x - 0.5)*(x - 0.5) + (y - 0.3)*(y - 0.3); exp(-r2 * 20)'
  []
[]

[FVKernels]
  [advection]
    type = FVAdvection
    variable = v
    velocity = '1 0.5 0'
  []
  [time]
    type = FVTimeKernel
    variable = v
  []
[]

[FVBCs]
  [fv_outflow]
    type = FVConstantScalarOutflowBC
    velocity = '1 0.5 0'
    variable = v
    boundary = 'right top'
  []
[]

[Executioner]
  type = Transient
  petsc_options = '-snes_converged_reason'
  num_steps = 10
  dt = 0.25
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
