[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 1.0
  zmin = 0.0
  zmax = 1.0
  nx = 2
  ny = 2
  nz = 2
  elem_type = HEX8
[]

[AuxVariables]
  [elem_id]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [elem_id_ak]
    type = ElementIDAux
    variable = elem_id
  []
[]

[VectorPostprocessors]
  [sample_points]
    type = PointValueSampler
    variable = "elem_id"
    points = "0.25 0.25 0.25
              0.75 0.25 0.25
              0.25 0.75 0.25
              0.75 0.75 0.25
              0.25 0.25 0.75
              0.75 0.25 0.75
              0.25 0.75 0.75
              0.75 0.75 0.75"
    sort_by = id
  []
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
