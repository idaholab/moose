# This example reproduces the libmesh vector_fe example 3 results

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  xmin = -1
  ymin = -1
  elem_type = QUAD9
[]

[Variables]
  [u]
    family = NEDELEC_ONE
    order = FIRST
  []
[]

[Kernels]
  [diff]
    type = VectorFEWave
    variable = u
    x_forcing_func = x_ffn
    y_forcing_func = y_ffn
  []
[]

[BCs]
  [bnd]
    type = VectorCurlPenaltyDirichletBC
    boundary = 'left right top bottom'
    penalty = 1e10
    function = sln
    variable = u
  []
[]

[Functions]
  [x_ffn]
    type = ParsedFunction
    expression = '(2*pi*pi + 1)*cos(pi*x)*sin(pi*y)'
  []
  [y_ffn]
    type = ParsedFunction
    expression = '-(2*pi*pi + 1)*sin(pi*x)*cos(pi*y)'
  []
  [sln]
    type = ParsedVectorFunction
    expression_x =  cos(pi*x)*sin(pi*y)
    expression_y = -sin(pi*x)*cos(pi*y)
    curl_z =  -2*pi*cos(pi*x)*cos(pi*y)
  []
[]

[Postprocessors]
  active = ''
  [L2Error]
    type = ElementVectorL2Error
    variable = u
    function = sln
  []
  [HCurlSemiError]
    type = ElementHCurlSemiError
    variable = u
    function = sln
  []
  [HCurlError]
    type = ElementHCurlError
    variable = u
    function = sln
  []
[]

[Preconditioning]
  [pre]
    type = SMP
  []
[]

[Executioner]
  type = Steady
  solve_type = LINEAR
  petsc_options_iname = -pc_type
  petsc_options_value = lu
[]

[Outputs]
  exodus = true
[]
