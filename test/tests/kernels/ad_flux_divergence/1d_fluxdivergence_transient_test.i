# This test solves a 1D transient heat equation
# The error is calculated by comparing to the analytical solution
# The problem setup and analytical solution are taken from:
# "Advanced Engineering Mathematics, 10th edition" by Erwin Kreyszig.
# Example 1 in section 12.6, page 561

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 160
  xmax = 80
[]

[Variables]
  [T]
  []
[]

[ICs]
  [T_IC]
    type = FunctionIC
    variable = T
    function = '100*sin(pi*x/80)'
  []
[]

[Kernels]
  [diff]
    type = ADFluxDivergence
    variable = T
  []
  [dt]
    type = CoefTimeDerivative
    variable = T
    Coefficient = 0.82064
  []
[]


[Materials]
  [diffusivity]
    type = ADGenericConstantMaterial
    prop_names = 'diffusivity'
    prop_values = '0.95'
  []

  [flux]
    type = ADFluxFromGradientMaterial
    flux = flux
    u = T
    diffusivity = diffusivity
  []
[]


[BCs]
  [sides]
    type = DirichletBC
    variable = T
    boundary = 'left right'
    value = 0
  []
[]

[Executioner]
  type = Transient
  dt = 1e-2
  end_time = 1
[]

[Functions]
  [analytical]
    type = ParsedFunction
    expression = '100*sin(pi*x/80)*exp(-0.95/(0.092*8.92)*pi^2/80^2*t)'
  []
[]

[Postprocessors]
  [error]
    type = NodalL2Error
    function = analytical
    variable = T
    outputs = console
  []
[]

[Outputs]
  file_base = transient_out
  exodus = true
[]
