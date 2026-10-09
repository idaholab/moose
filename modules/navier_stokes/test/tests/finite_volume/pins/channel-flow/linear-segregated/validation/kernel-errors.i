# Isolated LinearFVMomentumFriction constructor and assembly errors.

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
  []
[]

[Problem]
  linear_sys_names = 'u_sys'
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    solver_sys = u_sys
    initial_condition = 1.0
  []
[]

[FunctorMaterials]
  [darcy]
    type = GenericVectorFunctorMaterial
    prop_names = darcy
    prop_values = '1 1 1'
  []
  [forch]
    type = GenericVectorFunctorMaterial
    prop_names = forch
    prop_values = '1 1 1'
  []
[]

[LinearFVKernels]
  active = 'no_model'
  [no_model]
    type = LinearFVMomentumFriction
    variable = u
    momentum_component = x
  []
  [darcy_no_mu]
    type = LinearFVMomentumFriction
    variable = u
    momentum_component = x
    Darcy_name = darcy
  []
  [forch_no_rho]
    type = LinearFVMomentumFriction
    variable = u
    momentum_component = x
    Forchheimer_name = forch
    u = 1
    v = 0
    w = 0
  []
  [forch_no_u]
    type = LinearFVMomentumFriction
    variable = u
    momentum_component = x
    Forchheimer_name = forch
    rho = 1
    v = 0
    w = 0
  []
  [forch_no_v]
    type = LinearFVMomentumFriction
    variable = u
    momentum_component = x
    Forchheimer_name = forch
    rho = 1
    u = 1
    w = 0
  []
  [forch_no_w]
    type = LinearFVMomentumFriction
    variable = u
    momentum_component = x
    Forchheimer_name = forch
    rho = 1
    u = 1
    v = 0
  []
  [forch_zero_porosity]
    type = LinearFVMomentumFriction
    variable = u
    momentum_component = x
    Forchheimer_name = forch
    rho = 1
    u = 1
    v = 0
    w = 0
    porosity = 0
  []
  [source]
    type = LinearFVSource
    variable = u
    source_density = 1
  []
[]

[Executioner]
  type = Steady
  system_names = u_sys
[]

[Outputs]
  csv = false
  exodus = false
[]
