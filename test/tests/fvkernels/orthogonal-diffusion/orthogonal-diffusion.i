[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 32
    ny = 32
  []
[]

[Variables]
  [u]
    type = MooseVariableFVReal
  []
[]


[FVKernels]
  [diff]
    type = FVOrthogonalDiffusion
    variable = u
    coeff = 1
  []
[]

[FVBCs]
  [left]
    type = FVOrthogonalBoundaryDiffusion
    boundary = 'top'
    variable = u
    function = 0
    coeff = 1
  []
  [right]
    type = FVOrthogonalBoundaryDiffusion
    boundary = 'bottom'
    variable = u
    function = 1
    coeff = 1
  []
[]

[Executioner]
  solve_type = NEWTON
  type = Steady
[]

[Outputs]
  exodus = true
[]
