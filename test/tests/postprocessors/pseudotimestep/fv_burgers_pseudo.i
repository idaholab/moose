[Mesh]
  [gen_mesh]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = -1
    xmax = 1
    nx = 500
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
    function = '-1/(1+exp(-(x-z)/2/0.0005))'
  []
[]

[FVKernels]
  [burgers]
    type = FVBurgers1D
    variable = v
  []
  [difussion]
    type = FVDiffusion
    coeff= 0.0005
    variable = v
  []
  [time]
    type = FVTimeKernel
    variable = v
  []
[]

[FVBCs]
  [fv_burgers_outflow]
    type = FVBurgersOutflowBC
    variable = v
    boundary = 'left right'
  []
[]


[Postprocessors]
  [pseudotimestep]
    type = PseudoTimestep
    method = 'SER'
    initial_dt = 1
    alpha = 1.5
  []
[]

[Executioner]
  type = Transient
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  petsc_options = '-snes_converged_reason'
  num_steps = 5
  [TimeStepper]
    type = PostprocessorDT
    postprocessor = pseudotimestep
  []
[]

[Outputs]
  print_linear_residuals = false
  csv = true
[]
