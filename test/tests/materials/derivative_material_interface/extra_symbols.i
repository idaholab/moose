[Mesh]
  [gen]
   type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
  []
[]

[Variables]
  [dummy]
  []
[]

[GlobalParams]
  outputs = exodus
[]

[Materials]
  [x]
    type = DerivativeParsedMaterial
    property_name = Fx
    expression = x
    extra_symbols = x
  []
  [y]
    type = DerivativeParsedMaterial
    property_name = Fy
    expression = y
    extra_symbols = y
  []
  [z]
    type = DerivativeParsedMaterial
    property_name = Fz
    expression = z
    extra_symbols = z
  []
  [t]
    type = DerivativeParsedMaterial
    property_name = Ft
    expression = t
    extra_symbols = t
  []
  [dt]
    type = DerivativeParsedMaterial
    property_name = Fdt
    expression = dt
    extra_symbols = dt
  []
  [all]
    type = DerivativeParsedMaterial
    property_name = Fall
    expression = x*y*z+t*dt
    extra_symbols = 'z dt x y t'
  []
[]

[Executioner]
  type = Transient
  [TimeStepper]
    type = FunctionDT
    function = t+1
  []
  num_steps = 5
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Outputs]
  exodus = true
[]
