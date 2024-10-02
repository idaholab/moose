[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 2
  ny = 2
  nz = 2
  xmin = 0.0
  xmax = 4.0
  ymin = 0.0
  ymax = 6.0
  zmin = 0.0
  zmax = 10.0
[]

[Variables]
  [v1]
  []
[]

[Kernels]
  [time]
    type = TimeDerivative
    variable = v1
  []
  [source]
    type = BodyForce
    variable = v1
    function = 10
  []
[]

[Functions]
  [f1]
    type = ParsedFunction
    expression = '- 4 * t'
  []
  [f2]
    type = ConstantFunction
    value = 3
  []
[]

[AuxVariables]
  [v2]
    [AuxKernel]
      type = ParsedAux
      expression = '3 * t'
      use_xyzt = true
    []
  []
[]

[FunctorMaterials]
  [time_derivatives]
    type = ADGenericFunctorTimeDerivativeMaterial
    prop_names = 'f1dt f2dt v1dt v2dt'
    prop_values = 'f1 f2 v1 v2'
  []
[]

[Postprocessors]
  [f1_time]
    type = ElementExtremeFunctorValue
    functor = f1dt
    value_type = max
    execute_on = 'INITIAL'
  []
  [f2_time]
    type = ElementExtremeFunctorValue
    functor = f2dt
    value_type = max
    execute_on = 'INITIAL'
  []
  [v1_time]
    type = ElementExtremeFunctorValue
    functor = v1dt
    value_type = max
    # derivatives are not available on INITIAL and TIMESTEP_BEGIN
    execute_on = 'TIMESTEP_END'
  []
  [v2_time]
    type = ElementExtremeFunctorValue
    functor = v2dt
    value_type = max
    # derivatives are not available on INITIAL and TIMESTEP_BEGIN
    execute_on = 'TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Outputs]
  csv = true
[]
