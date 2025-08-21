epsilon = 1.0
kappa1tomax = 0.4
k = 100
n1 = 1.46

nu1 = 1e-4
nu_max = 1e17

[Mesh]
  [gmesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 1
    xmin = -0.5
    xmax = 0.5
  []
[]

[Functions]
[force_1]
  type = ParsedFunction
  expression = '-6.31957783783189e-10*pi^5*(-sin(500*pi*t)*cos(x*pi) + 10)^4 - 94.448388749631*sin(500*pi*t)*cos(0.5*x) + 688.96777499262*cos(x) + 200.0'
[]
[exact_1]
  type = ParsedFunction
  expression = '-200*sin(500*pi*t)*cos(0.5*x) + 1000*cos(x) + 500'
[]
[force_2]
  type = ParsedFunction
  expression = '-6.31957783783189e-10*pi^5*(-sin(500*pi*t)*cos(x*pi) + 10)^4 - 86.3472341966131*sin(500*pi*t)*cos(0.5*x) + 3907.77873572905*cos(2*x)'
[]
[exact_2]
  type = ParsedFunction
  expression = '-100*sin(500*pi*t)*cos(0.5*x) + 500*cos(2*x)'
[]
[force_Temp]
  type = ParsedFunction
  expression = '-25.5445566926545*sin(500*pi*t)*cos(0.5*x) - 100.0*pi^2*sin(500*pi*t)*cos(x*pi) + 188.448934372847*cos(x) + 1289.76879792097*cos(2*x) - 500*pi*cos(x*pi)*cos(500*pi*t)'
[]
[exact_Temp]
  type = ParsedFunction
  expression = '-sin(500*pi*t)*cos(x*pi) + 10'
[]
[]

[Problem]
  nl_sys_names = 'Temp
                  psi1
                  psi2'

  verbose_setup = true
[]

[Variables]
  [Temp]
    type = MooseVariableFVReal
    initial_condition = '10'
    solver_sys = 'Temp'
  []

  [psi1]
    type = MooseVariableFVReal
    solver_sys = 'psi1'
    initial_condition = '1500'
  []

  [psi2]
    type = MooseVariableFVReal
    solver_sys = 'psi2'
    initial_condition = '500'
  []
[]

[FVKernels]
  [diffusion1]
    type = FVSP3ThermalRadiationDiffusion
    variable = psi1
    epsilon = ${epsilon}
    kappa = ${kappa1tomax}
    order = first
  []

  [diffusion2]
    type = FVSP3ThermalRadiationDiffusion
    variable = psi2
    epsilon = ${epsilon}
    kappa = ${kappa1tomax}
    order = second
  []

  [sink1]
    type = FVSP3ThermalRadiationSourceSink
    variable = psi1
    T = 'Temp'
    nu = ${nu1}
    nu_low =${nu1}
    nu_high = ${nu_max}
    refraction_index = ${n1}
    kappa = ${kappa1tomax}
  []

  [sink2]
    type = FVSP3ThermalRadiationSourceSink
    variable = psi2
    T = 'Temp'
    nu = ${nu1}
    nu_low = ${nu1}
    nu_high = ${nu_max}
    refraction_index = ${n1}
    kappa = ${kappa1tomax}
  []

  [energy_source]
    type = FVSP3TemperatureSourceSink
    variable = Temp
    absorptivities = '${kappa1tomax}'
    psi_1 = 'psi1'
    psi_2 = 'psi2'
  []

  [energy_time]
    type = FVTimeKernel
    variable = Temp
  []

  [energy_diffusion]
    type = FVDiffusion
    variable = Temp
    coeff = ${k}
  []

  [force1]
    type = FVBodyForce
    variable = psi1
    function = force_1
  []
  [force2]
    type = FVBodyForce
    variable = psi2
    function = force_2
  []

  [forceTemp]
    type = FVBodyForce
    variable = Temp
    function = force_Temp
  []
[]

[FVBCs]
  [BC_1]
    type = FVFunctionDirichletBC
    boundary = 'left right'
    variable = psi1
    function = 'exact_1'
  []
  [BC_2]
    type = FVFunctionDirichletBC
    boundary = 'left right'
    variable = psi2
    function = 'exact_2'
  []
  [BC_Temp]
    type = FVFunctionDirichletBC
    boundary = 'left right'
    variable = Temp
    function = 'exact_Temp'
  []
[]

[Postprocessors]
#   [error_1]
#     type = ElementL2Error
#     function = exact_1
#     variable = psi1
#   []
#   [error_2]
#     type = ElementL2Error
#     function = exact_2
#     variable = psi2
#   []
  [error]
    type = ElementL2Error
    function = exact_Temp
    variable = Temp
  []
  [h]
    type = AverageElementSize
  []
[]

[Executioner]
  type = Transient
  solve_type = 'Newton'
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 500'

  start_time = 0.0
  dt = 5e-7
  end_time = 5e-4
[]

[Outputs]
    exodus = true
    csv = true
[]
