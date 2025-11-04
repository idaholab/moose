[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 4
    ny = 4
    elem_type = QUAD4
  []
  displacements = 'disp_x disp_y'
[]

[AuxVariables]
  [disp_x]
  []
  [disp_y]
  []
[]

[AuxKernels]
  [disp_x_aux]
    type = ParsedAux
    variable = disp_x
    expression = '10 * x/80'
    use_xyzt = true
    execute_on = INITIAL
  []

  [disp_y_aux]
    type = ParsedAux
    variable = disp_y
    expression = '10 * y/80'
    use_xyzt = true
    execute_on = INITIAL
  []
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
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

[Postprocessors]
  [print_displaced_volume]
    type = DisplacedMeshBlockVolume
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
