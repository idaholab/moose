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
    type = Diffusion
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
    type = MaterialRealAux
    variable = Hw
    property = Hw
  []
[]

[Materials]
  [props]
    type = GenericConstantMaterial
    prop_names = 'rho vel D_h k mu cp T T_wall'
    prop_values = '1000 0.1 0.1 0.001 0.1 12 300 310'
  []

  [Hw_material]
    type = WallHeatTransferCoefficient3EqnDittusBoelterMaterial
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
