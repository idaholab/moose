# This is a test of the ConvectiveFluxFunction BC.
# There is a single 1x1 element with a prescribed temperature
# on the left side and a convective flux BC on the right side.
# The temperature on the left is 100, and the far-field temp is 200.
# The conductance of the body (conductivity * length) is 10
#
# If the conductance in the BC is also 10, the temperature on the
# right side of the solid element should be 150 because half of the
# temperature drop should occur over the body and half in the BC.
#
# The integrated flux is deltaT * conductance, or -50 * 10 = -500.
# The negative sign indicates that heat is going into the body.
#
# The conductance is defined multiple ways using this input, and
# as long as it evaluates to 10, the result described above will
# be obtained.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
[]

[Problem]
  extra_tag_vectors = 'bcs'
[]

[Variables]
  [temp]
    initial_condition = 100.0
  []
[]

[AuxVariables]
  [flux]
  []
[]

[AuxKernels]
  [flux]
    type = TagVectorAux
    variable = flux
    v = temp
    vector_tag = 'bcs'
    execute_on = timestep_end
  []
[]

[Kernels]
  [heat_conduction]
    type = HeatConduction
    variable = temp
  []
[]

[Materials]
  [thermal]
    type = HeatConductionMaterial
    thermal_conductivity = 10.0
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = temp
    boundary = left
    value = 100.0
  []
  [right]
    type = ConvectiveFluxFunction
    variable = temp
    boundary = right
    T_infinity = 200.0
    coefficient = 10.0 #This will behave as described in the header of this file if this evaluates to 10
    extra_vector_tags = 'bcs'
  []
[]

[Postprocessors]
  [integrated_flux]
    type = NodalSum
    variable = flux
    boundary = right
  []
[]

[Executioner]
  type = Transient
  start_time = 0.0
  end_time = 1.0
  dt = 1.0
  nl_rel_tol=1e-12
[]

[Outputs]
  csv = true
[]
