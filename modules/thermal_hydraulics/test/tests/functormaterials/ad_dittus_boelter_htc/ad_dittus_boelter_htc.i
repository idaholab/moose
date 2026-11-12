# Evaluates ADDittusBoelterHTCFunctorMaterial for a forward-flow segment and for the
# mirror-image reverse-flow segment (negated mass flow rate, inlet and outlet neighbor
# temperatures swapped). The upwinded inlet temperature, and hence the coefficient,
# must be the same in both cases.

Di = 0.05
flow_area = '${fparse pi / 4 * Di^2}'
wetted_perimeter = '${fparse pi * Di}'

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables]
  [m_fwd]
    family = SCALAR
    initial_condition = 1.0
  []
  [m_rev]
    family = SCALAR
    initial_condition = -1.0
  []
  [T_fluid]
    family = SCALAR
    initial_condition = 320
  []
  [T_a]
    family = SCALAR
    initial_condition = 300
  []
  [T_b]
    family = SCALAR
    initial_condition = 340
  []
[]

[FluidProperties]
  [water]
    type = Water97FluidProperties
  []
[]

[FunctorMaterials]
  [h_fwd]
    type = ADDittusBoelterHTCFunctorMaterial
    mass_flow_rate = m_fwd
    fluid_properties = water
    T_fluid = T_fluid
    T_fluid_inlet = T_a
    T_fluid_outlet = T_b
    reference_pressure = 101300
    flow_area = ${flow_area}
    wetted_perimeter = ${wetted_perimeter}
  []
  [h_rev]
    type = ADDittusBoelterHTCFunctorMaterial
    mass_flow_rate = m_rev
    fluid_properties = water
    T_fluid = T_fluid
    T_fluid_inlet = T_b
    T_fluid_outlet = T_a
    reference_pressure = 101300
    flow_area = ${flow_area}
    wetted_perimeter = ${wetted_perimeter}
  []
[]

[Postprocessors]
  [h_fwd]
    type = ADElementExtremeFunctorValue
    functor = h_fwd
    execute_on = 'INITIAL'
  []
  [h_rev]
    type = ADElementExtremeFunctorValue
    functor = h_rev
    execute_on = 'INITIAL'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  execute_on = 'INITIAL'
[]
