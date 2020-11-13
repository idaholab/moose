[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = -1
  xmax =  1
  ymin = -1
  ymax =  1
  zmin = -1
  zmax =  1
  nx = 2
  ny = 2
  nz = 2
[../]

[Variables]
  [./u]
    family = LAGRANGE_VEC
    order = first
  [../]
[]

[Kernels]
  [./none]
    type = VectorDiffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = VectorDirichletBC
    variable = u
    boundary = left
    values = '0 0 0'
  [../]
  [./right]
    type = VectorDirichletBC
    variable = u
    boundary = right
    values = '1 2 3'
  [../]
[]

[AuxVariables]
  [./u_mag]
  [../]
[]

[AuxKernels]
  [./calc_u_mag]
    type = VectorVariableMagnitudeAux
    variable = u_mag # the auxvariable to compute
    vector_variable = u # vector variable to compute from
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
