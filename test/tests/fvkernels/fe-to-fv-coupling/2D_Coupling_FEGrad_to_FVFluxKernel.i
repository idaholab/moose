#Checked

[GlobalParams]
  advected_interp_method = average
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 10
    ny = 10
  []
[]

[Problem]
  type = FEProblem
[]

[Variables]
  [u]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  []

  [v]
  []
[]

[FVKernels]
  [u_diffusion]
    type = FVDiffusion
    variable = u
    coeff = 1.0
  []
  [u_advection]
    type = FVCoupledAdvection
    variable = u
    v = v
    #advected_interp_method = average
  []
  [u_source]
    type = FVBodyForce
    variable = u
    function = 'u_source'
  []
[]

[Kernels]
  #Potential Equations
  [u_diffusion]
    type = Diffusion
    variable = v
  []
  [u_source]
    type = ADBodyForce
    variable = v
    function = 'v_source'
  []
[]

[Functions]
  [u_fun]
    type = ParsedFunction
    value = 'cos(2*x*pi)*cos(2*y*pi)'
  []
  [v_fun]
    type = ParsedFunction
    value = 'sin(2*x*pi)*sin(2*y*pi)'
  []
  [u_source]
    type = ParsedFunction
    value = '-16*pi^2*sin(2*x*pi)*sin(2*y*pi)*cos(2*x*pi)*cos(2*y*pi) + 8*pi^2*cos(2*x*pi)*cos(2*y*pi)'
  []
  [v_source]
    type = ParsedFunction
    value = '8*pi^2*sin(2*x*pi)*sin(2*y*pi)'
  []
[]

[FVBCs]
  [u_BC]
    type = FVFunctionDirichletBC
    variable = u
    function = 'u_fun'
    boundary = '0 1 2 3'
  []
[]

[BCs]
  [v_BC]
    type = FunctionDirichletBC
    variable = v
    function = 'v_fun'
    boundary = '0 1 2 3'
  []
[]

[Postprocessors]
  [u_l2Error]
    type = ElementCenterL2Error
    variable = u
    function = u_fun
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []

  [v_l2Error]
    type = ElementL2Error
    variable = v
    function = v_fun
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []

  [h]
    type = AverageElementSize
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
[]

[Preconditioning]
  active = 'smp'
  [smp]
    type = SMP
    full = true
  []

  [fdp]
    type = FDP
    full = true
  []
[]

[Executioner]
  type = Steady

  petsc_options = '-snes_converged_reason -snes_linesearch_monitor'
  solve_type = NEWTON
  line_search = none
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu NONZERO 1.e-10'
[]

[Outputs]
  exodus = true
  csv = true
[]
