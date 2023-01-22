[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 10
  nz = 1
  xmin = -5
  xmax = 5
  ymin = -5
  ymax = 5
  zmin = 0
  zmax = 1
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [./c]
  [../]
[]

[AuxVariables]
  [./d]
  [../]
[]

[AuxKernels]
  [./d]
    type = FunctionAux
    variable = d
    function = set_d
    execute_on = initial
  [../]
[]

[Functions]
  [./set_d]
    type = ParsedFunction
    expression = 'r := sqrt(x * x + y * y); r'
  [../]
[]

[VectorPostprocessors]
  [./average]
    type = CylindricalAverage
    variable = d
    radius = 5
    bin_number = 10
    origin = '0 0 0'
    cylinder_axis = '0 0 1'
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  execute_on = 'initial timestep_end'
  csv = true
[]
