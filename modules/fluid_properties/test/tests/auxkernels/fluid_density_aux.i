[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./pressure]
  [../]
  [./temperature]
  [../]
  [./density]
  [../]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  [./pressure_ak]
    type = ConstantAux
    variable = pressure
    value = 10e6
  [../]
  [./temperature_ak]
    type = ConstantAux
    variable = temperature
    value = 400.0
  [../]
  [./density]
    type = FluidDensityAux
    variable = density
    fp = eos
    p = pressure
    T = temperature
  [../]
[]

[FluidProperties]
  [./eos]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    q = -1167e3
    q_prime = 0.0
    p_inf = 1e9
    cv = 1816.0
  [../]
[]

[BCs]
  [./left_u]
    type = DirichletBC
    variable = u
    boundary = 0
    value = 1
  [../]

  [./right_u]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 2
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
