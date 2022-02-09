[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  allow_renumbering = false
[]

[Variables]
  [arhoA]
  []
  [arhouA]
  []
  [arhoEA]
  []
[]

[AuxVariables]
  [Re]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [Re_ak]
    type = ADMaterialRealAux
    variable = Re
    property = my_Re
  []
[]

[Materials]
  [rho_mat]
    type = ADConstantMaterial
    property_name = rho
    value = 1000
  []
  [vel_mat]
    type = ADConstantMaterial
    property_name = vel
    value = 5
  []
  [D_h_mat]
    type = ADConstantMaterial
    property_name = D_h
    value = 0.002
  []
  [mu_mat]
    type = ADConstantMaterial
    property_name = mu
    value = 0.1
  []

  [Re_material]
    type = ADReynoldsNumberMaterial
    Re = my_Re
    rho = rho
    vel = vel
    D_h = D_h
    mu = mu
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [Re]
    type = ElementalVariableValue
    elementid = 0
    variable = Re
  []
[]

[Outputs]
  csv = true
  execute_on = timestep_end
[]
