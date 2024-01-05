# Test for DivField and GradField kernels and VectorDivPenaltyDirichletBC bcs.
# This test uses Raviart-Thomas elements to solve a model div-grad problem
# in H(div). The problem is simply a div-grad formulation, u = -grad(p), and
# div(u) = f, of the standard Poisson problem div(grad(p)) = -f.
# Manufactured solution: p = cos(k*x)*sin(k*y)*cos(k*z).

k = asin(1)

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 6
    ny = 6
    nz = 6
    xmax =  1
    ymax =  1
    zmax =  1
    xmin = -1
    ymin = -1
    zmin = -1
    elem_type = HEX27
  []
[]

[Variables]
  [u]
    family = RAVIART_THOMAS
    order = FIRST
  []
  [p]
    family = MONOMIAL
    order = CONSTANT
  []
  [lambda]
    family = SCALAR
    order = FIRST
  []
[]

[Functions]
  [f]
    type = ParsedVectorFunction
    expression_x =  ${k}*sin(${k}*x)*sin(${k}*y)*cos(${k}*z)
    expression_y = -${k}*cos(${k}*x)*cos(${k}*y)*cos(${k}*z)
    expression_z =  ${k}*cos(${k}*x)*sin(${k}*y)*sin(${k}*z)
    div = ${Mesh/gmg/dim}*${k}^2*cos(${k}*x)*sin(${k}*y)*cos(${k}*z)
  []
[]

[Kernels]
  [coefficient]
    type = VectorFunctionReaction
    variable = u
    sign = negative
  []
  [gradient]
    type = GradField
    variable = u
    coupled_scalar_variable = p
  []
  [divergence]
    type = DivField
    variable = p
    coupled_vector_variable = u
  []
  [forcing]
    type = BodyForce
    variable = p
    function = ${Functions/f/div}
  []
  [mean_zero_p]
    type = ScalarLagrangeMultiplier
    variable = p
    lambda = lambda
  []
[]

[ScalarKernels]
  [constraint]
    type = AverageValueConstraint
    variable = lambda
    pp_name = PP
    value = 0.0
  []
[]

[BCs]
  [sides]
    type = VectorDivPenaltyDirichletBC
    variable = u
    function = f
    penalty = 1e8
    boundary = 'top bottom left right front back'
  []
[]

[Postprocessors]
  active = PP
  [PP]
    type = ElementIntegralVariablePostprocessor
    variable = p
    execute_on = linear
  []
  [L2Error]
    type = ElementVectorL2Error
    variable = u
    function = f
  []
  [HDivSemiError]
    type = ElementHDivSemiError
    variable = u
    function = f
  []
  [HDivError]
    type = ElementHDivError
    variable = u
    function = f
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
  petsc_options_iname = '-pc_type -ksp_rtol -ksp_norm_type'
  petsc_options_value = '  jacobi     1e-12 preconditioned'
[]

[Outputs]
  exodus = true
[]
