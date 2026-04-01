# Test for DivDivField and VectorDivPenaltyDirichletBC bcs which reproduces
# the results for Vector Finite Elements Example 10 in libMesh.
# This test uses Raviart-Thomas elements to solve a model grad-div problem
# in H(div). We solve -grad(a div(u)) + b u = f for u given f.
# Manufactured solution: u = F = {cos(k*x)*sin(k*y), cos(k*y)*sin(k*x), 0}.
#                        f = (2*a*k^2 + b)*F

a = 1
b = 1
k = acos(-1)

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 12
    ny = 12
    xmax =  1
    ymax =  1
    xmin = -1
    ymin = -1
    elem_type = HEX27
  []
[]

[Variables]
  [u]
    family = RAVIART_THOMAS
    order = FIRST
  []
[]

[Functions]
  [F]
    type = ParsedVectorFunction
    expression_x = cos(${k}*x)*sin(${k}*y)
    expression_y = cos(${k}*y)*sin(${k}*x)
    expression_z = 0
    div = -2*${k}*sin(${k}*x)*sin(${k}*y)
  []
  [f]
    type = ParsedVectorFunction
    expression_x = (2*${a}*${k}^2+${b})*cos(${k}*x)*sin(${k}*y)
    expression_y = (2*${a}*${k}^2+${b})*cos(${k}*y)*sin(${k}*x)
    expression_z = 0
  []
[]

[Kernels]
  [divergence]
    type = DivDivField
    variable = u
    coeff = ${a}
  []
  [coefficient]
    type = VectorFunctionReaction
    variable = u
    function = ${b}
  []
  [forcing]
    type = VectorBodyForce
    variable = u
    function = f
  []
[]

[BCs]
  [sides]
    type = VectorDivPenaltyDirichletBC
    variable = u
    function = F
    penalty = 1e10
    boundary = 'top bottom left right front back'
  []
[]

[Postprocessors]
  [L2Error]
    type = ElementVectorL2Error
    variable = u
    function = F
  []
  [HDivSemiError]
    type = ElementHDivSemiError
    variable = u
    function = F
  []
  [HDivError]
    type = ElementHDivError
    variable = u
    function = F
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = LINEAR
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_rtol -ksp_norm_type'
  petsc_options_value = '   hypre            ads     1e-14 preconditioned'
[]

[Outputs]
  exodus = true
[]
