# Tests calculation of first and second time derivative
# of a coupled vector variable in a material
# a_vec(x,y,z,t) = [t*(t*x + y), t*y, 0]
# a_vec_dot(x,y,z,t) = [2*t*x + y, y, 0]
# a_vec_dot_dot(x,y,z,t) = [2*x, 0, 0]
#
# IMPORTANT NOTE:
# Currently, this test produces a_vec_dot and a_vec_dot_dot that contains oscillations over time.
# This is a known by-product of Newmark Beta time integration (see the Newmark Beta documentation),
# but as of Summer 2019, there is no alternative time integrator in MOOSE that can dampen these
# oscillations. This test is used as coverage for the function call coupledVectorDotDot.

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 4
  ymin = 0
  ymax = 4
  nx = 8
  ny = 8
[]

[Functions]
  [a_fn]
    type = ParsedVectorFunction
    expression_x = 't * (t * x + y)'
    expression_y = 't * y'
    expression_z = 0
  []
[]

[AuxVariables]
  [a]
    family = LAGRANGE_VEC
    order = FIRST
  []
[]

[AuxKernels]
  [a_ak]
    type = VectorFunctionAux
    variable = a
    function = a_fn
  []
[]

[Materials]
  [cm]
    type = VectorCoupledValuesMaterial
    variable = a
  []
[]

[Variables]
  [u]   # u is zero
    family = LAGRANGE_VEC
    order = FIRST
  []
[]

[Kernels]
  [td]
    type = VectorTimeDerivative
    variable = u
  []
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 3

  [TimeIntegrator]
    type = NewmarkBeta
  []
[]

[Outputs]
  [./out]
    type = Exodus
    output_material_properties = true
    show_material_properties = 'a_value a_dot a_dot_dot a_dot_du a_dot_dot_du'
    execute_on = 'TIMESTEP_END'
  [../]
[]
