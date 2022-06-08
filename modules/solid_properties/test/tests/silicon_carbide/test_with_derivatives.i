[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  elem_type = QUAD4
[]

[Variables]
  [u]
    initial_condition = 1000.0
  []
[]

[AuxVariables]
  [T]
    initial_condition = 923.15
  []
  [cp]
    family = MONOMIAL
    order = CONSTANT
  []
  [k]
    family = MONOMIAL
    order = CONSTANT
  []
  [rho]
    family = MONOMIAL
    order = CONSTANT
  []
  [dcp_dT]
    family = MONOMIAL
    order = CONSTANT
  []
  [dk_dT]
    family = MONOMIAL
    order = CONSTANT
  []
  [drho_dT]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [cp]
    type = MaterialRealAux
    variable = cp
    property = cp_solid
  []
  [k]
    type = MaterialRealAux
    variable = k
    property = k_solid
  []
  [rho]
    type = MaterialRealAux
    variable = rho
    property = rho_solid
  []
  [dcp_dT]
    type = MaterialRealAux
    variable = dcp_dT
    property = dcp_solid/dT
  []
  [dk_dT]
    type = MaterialRealAux
    variable = dk_dT
    property = dk_solid/dT
  []
  [drho_dT]
    type = MaterialRealAux
    variable = drho_dT
    property = drho_solid/dT
  []
[]

[Materials]
  [sp_mat]
    type = ThermalSiliconCarbideProperties
    temperature = T
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 1000.0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = 'right'
    value = 500.0
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
