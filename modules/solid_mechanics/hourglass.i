[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    elem_type = QUAD4
  []
[]

[Variables]
  [disp_x]
    [InitialCondition]
      type = FunctionIC
      function = 'y - (x-0.5)'
      # function = x
    []
  []
  [disp_y]
    [InitialCondition]
      type = FunctionIC
      function = 0 # y
    []
  []
[]

[Kernels]
  [hourglass]
    type = HourglassCorrectionQuad4
    variable = disp_x
    displacements = 'disp_x disp_y'
  []
[]

[Problem]
  kernel_coverage_check = FALSE
[]

[Executioner]
  type = Steady
  [Quadrature]
    type = GAUSS
    order = CONSTANT
  []
[]
