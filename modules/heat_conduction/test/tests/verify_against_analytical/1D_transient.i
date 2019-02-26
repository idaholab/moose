# This test solves a 1D transient heat equation
# The error is caclulated by comparing to the analytical solution
# The problem setup and analytical solution are taken from "Advanced Engineering
# Mathematics, 10th edition" by Erwin Kreyszig.
# http://www.amazon.com/Advanced-Engineering-Mathematics-Erwin-Kreyszig/dp/0470458364
# It is Example 1 in section 12.6 on page 561

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 160
  xmax = 80
[]

[Variables]
  [./T]
  [../]
[]

[ICs]
  [./T_IC]
    type = FunctionIC
    variable = T
    function = '100*sin(pi*x/80)'
  [../]
[]

[Kernels]
  [./HeatDiff]
    type = HeatConduction
    variable = T
  [../]
  [./HeatTdot]
    type = HeatConductionTimeDerivative
    variable = T
  [../]
[]

[BCs]
  [./sides]
    type = DirichletBC
    variable = T
    boundary = 'left right'
    value = 0
  [../]
[]

[Materials]
  [./k]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '0.95' #copper in cal/(cm sec C)
  [../]
  [./cp]
    type = GenericConstantMaterial
    prop_names = 'specific_heat'
    prop_values = '0.092' #copper in cal/(g C)
  [../]
  [./rho]
    type = GenericConstantMaterial
    prop_names = 'density'
    prop_values = '8.92' #copper in g/(cm^3)
  [../]
[]

[Postprocessors]
  [./error]
    type = NodalL2Error
    function = '100*sin(pi*x/80)*exp(-0.95/(0.092*8.92)*pi^2/80^2*t)'
    variable = T
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2
  l_tol = 1e-6
  dt = 2
  end_time = 100
[]

[Outputs]
  exodus = true
[]
