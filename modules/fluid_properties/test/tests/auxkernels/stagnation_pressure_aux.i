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
  [./specific_internal_energy]
  [../]
  [./specific_volume]
  [../]
  [./velocity]
  [../]
  [./stagnation_pressure]
  [../]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  [./specific_internal_energy_ak]
    type = ConstantAux
    variable = specific_internal_energy
    value = 1026.2e3
  [../]
  [./specific_volume_ak]
    type = ConstantAux
    variable = specific_volume
    value = 0.0012192
  [../]
  [./velocity_ak]
    type = ConstantAux
    variable = velocity
    value = 10.0
  [../]
  [./stagnation_pressure_ak]
    type = StagnationPressureAux
    variable = stagnation_pressure
    e = specific_internal_energy
    v = specific_volume
    vel = velocity
    fp = eos
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

  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]
