[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  allow_renumbering = false
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = ADDiffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[AuxVariables]
  [Hw]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [Hw_ak]
    type = ADMaterialRealAux
    variable = Hw
    property = Hw
  []
[]

[Materials]
  [props]
    type = ADGenericConstantMaterial
    prop_names = 'rho vel k mu cp T T_wall D_h'
    prop_values = '1000 0.1 0.001 0.1 12 300 310 0.1'
  []

  [Hw_material]
    type = ADWallHeatTransferCoefficient3EqnDittusBoelterMaterial
    rho = rho
    vel = vel
    D_h = D_h
    k = k
    mu = mu
    cp = cp
    T = T
    T_wall = T_wall
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [Hw]
    type = ElementalVariableValue
    elementid = 0
    variable = Hw
  []
[]

[Outputs]
  csv = true
  execute_on = timestep_end
[]
