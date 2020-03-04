[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  allow_renumbering = false
[]

[Variables]
  [./u]
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
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[AuxVariables]
  [./Re]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[AuxKernels]
  [./Re_ak]
    type = MaterialRealAux
    variable = Re
    property = Re
  [../]
[]

[Materials]
  [./props]
    type = GenericConstantMaterial
    prop_names = 'rho vel D_h mu'
    prop_values = '1000 0.1 0.1 0.1'
  [../]

  [./Re_material]
    type = ReynoldsNumberMaterial
    rho = rho
    vel = vel
    D_h = D_h
    mu = mu
  [../]
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [./Re]
    type = ElementalVariableValue
    elementid = 0
    variable = Re
  [../]
[]

[Outputs]
  csv = true
  execute_on = timestep_end
[]
