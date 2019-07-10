# This test solves a 1D transient heat equation
# The error is calculated by comparing to the analytical solution
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
  [./diff]
    type = MatDiffusion
    variable = T
    diffusivity = 0.95
  [../]
  [./dt]
    type = CoefTimeDerivative
    variable = T
    Coefficient = 0.82064
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

[Executioner]
  type = Transient
  dt = 1e-2
  end_time = 1
[]

[Postprocessors]
  [./error]
    type = NodalL2Error
    function = '100*sin(pi*x/80)*exp(-0.95/(0.092*8.92)*pi^2/80^2*t)'
    variable = T
    outputs = console
  [../]
[]

[Outputs]
  exodus = true
[]
