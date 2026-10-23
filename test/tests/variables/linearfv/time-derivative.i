# The time derivative a linear finite volume variable reports through dot() has to be the discrete
# operator LinearFVTimeDerivative assembles, whatever the time integrator. The equation here is
#
#   du/dt = 3 t^2
#
# with no spatial terms, so each cell carries the scalar recurrence of the integrator exactly and
# the assembled derivative equals the source to round-off after every solve. The functor material
# below evaluates u.dot() and the postprocessor reports the largest difference from the source over
# the mesh. Under implicit Euler a hand-rolled (u - u_old)/dt would pass this too; under BDF2 it
# would miss by the difference between the two schemes, which is order one at this time step.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
[]

[Problem]
  linear_sys_names = 'u_sys'
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    solver_sys = 'u_sys'
  []
[]

[LinearFVKernels]
  [time]
    type = LinearFVTimeDerivative
    variable = u
  []
  [source]
    type = LinearFVSource
    variable = u
    source_density = rate
  []
[]

[Functions]
  [rate]
    type = ParsedFunction
    expression = '3*t^2'
  []
[]

[FunctorMaterials]
  [u_dot]
    type = GenericFunctorTimeDerivativeMaterial
    prop_names = 'u_dot'
    prop_values = 'u'
  []
  [mismatch]
    type = ParsedFunctorMaterial
    property_name = mismatch
    expression = 'abs(u_dot - rate)'
    functor_names = 'u_dot rate'
  []
[]

[Postprocessors]
  [max_mismatch]
    type = ElementExtremeFunctorValue
    functor = mismatch
    value_type = max
    execute_on = 'TIMESTEP_END'
  []
  [u_avg]
    type = ElementAverageValue
    variable = u
    execute_on = 'TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  system_names = u_sys
  l_tol = 1e-12
  dt = 1
  num_steps = 3
  scheme = 'bdf2'
[]

[Outputs]
  csv = true
[]
