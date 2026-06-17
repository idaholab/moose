# Jacobian test for ADPorousFlowEnergyTimeDerivative
# 0 phases (rock heat only), constant porosity
# AD path: Jacobian verified against finite differences via PetscJacobianTester
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  xmin = 0
  xmax = 1
  ny = 1
  ymin = 0
  ymax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [temp]
  []
[]

[ICs]
  [temp]
    type = RandomIC
    variable = temp
    max = 1.0
    min = 0.0
  []
[]

[Kernels]
  [energy_dot]
    type = ADPorousFlowEnergyTimeDerivative
    variable = temp
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'temp'
    number_fluid_phases = 0
    number_fluid_components = 0
  []
[]

[Materials]
  [temperature]
    type = ADPorousFlowTemperature
    temperature = temp
  []
  [porosity]
    type = ADPorousFlowPorosityConst
    porosity = 0.1
  []
  [rock_heat]
    type = ADPorousFlowMatrixInternalEnergy
    specific_heat_capacity = 1.1
    density = 0.5
  []
[]

[Preconditioning]
  [check]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -snes_type'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000 test'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 1
[]

[Outputs]
  exodus = false
[]
