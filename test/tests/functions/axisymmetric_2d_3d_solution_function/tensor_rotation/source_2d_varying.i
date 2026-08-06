# Source: 2D RZ with spatially-varying tensor field. Companion to consume_3d_varying.i.
#
# T_rr = r, T_yy = y, T_ry = 0.1*r, T_tt = -r
#
# These give closed-form Cartesian components after rotation; consume_3d_varying.i
# compares observed (tensor-mode Axisymmetric2D3DSolutionFunction) to those
# analytical references at every node.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    xmin = 1
    xmax = 6
    ny = 4
    ymin = 0
    ymax = 4
  []
  coord_type = RZ
  rz_coord_axis = y
[]

[Variables]
  [u]
    family = LAGRANGE
    order = FIRST
    initial_condition = 0
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [bottom]
    type = DirichletBC
    variable = u
    boundary = bottom
    value = 0
  []
[]

[Functions]
  [Trr_fn]
    type = ParsedFunction
    expression = 'x'
  []
  [Tyy_fn]
    type = ParsedFunction
    expression = 'y'
  []
  [Try_fn]
    type = ParsedFunction
    expression = '0.1*x'
  []
  [Ttt_fn]
    type = ParsedFunction
    expression = '-x'
  []
[]

[AuxVariables]
  [T_rr]
    family = LAGRANGE
    order = FIRST
  []
  [T_yy]
    family = LAGRANGE
    order = FIRST
  []
  [T_ry]
    family = LAGRANGE
    order = FIRST
  []
  [T_tt]
    family = LAGRANGE
    order = FIRST
  []
[]

[AuxKernels]
  [Trr_aux]
    type = FunctionAux
    variable = T_rr
    function = Trr_fn
    execute_on = INITIAL
  []
  [Tyy_aux]
    type = FunctionAux
    variable = T_yy
    function = Tyy_fn
    execute_on = INITIAL
  []
  [Try_aux]
    type = FunctionAux
    variable = T_ry
    function = Try_fn
    execute_on = INITIAL
  []
  [Ttt_aux]
    type = FunctionAux
    variable = T_tt
    function = Ttt_fn
    execute_on = INITIAL
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
