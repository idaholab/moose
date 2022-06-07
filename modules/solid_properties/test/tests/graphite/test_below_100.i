[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 2
  elem_type = QUAD4
[]

[Variables]
  [./u]
    initial_condition = 1000.0
  [../]
[]

[AuxVariables]
  [./T]
    initial_condition = 368.15
  [../]
  [./cp]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./dcp_dT]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./k]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./dk_dT]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./rho]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./drho_dT]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[AuxKernels]
  [./cp]
    type = MaterialRealAux
    variable = cp
    property = cp_solid
  [../]
  [./dcp_dT]
    type = MaterialRealAux
    variable = dcp_dT
    property = dcp_solid/dT
  [../]
  [./k]
    type = MaterialRealAux
    variable = k
    property = k_solid
  [../]
  [./dk_dT]
    type = MaterialRealAux
    variable = dk_dT
    property = dk_solid/dT
  [../]
  [./rho]
    type = MaterialRealAux
    variable = rho
    property = rho_solid
  [../]
  [./drho_dT]
    type = MaterialRealAux
    variable = drho_dT
    property = drho_solid/dT
  [../]
[]

[Materials]
  [./sp_mat]
    type = ThermalGraphiteProperties
    temperature = T
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 1000.0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 'right'
    value = 500.0
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
