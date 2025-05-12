# Test for coupledVectorValuesOld for nodal variables

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 10
    ny = 1
    nz = 1
  []
[]

[Kernels]
  [time_deriv]
    type = VectorTimeDerivative
    variable = var
  []
  [bodyf]
    type = VectorBodyForce
    variable = var
    function_x = '-1'
    function_y = '-1'
    function_z = '-1'
  []
[]

[ICs]
  [ics]
    type = VectorFunctionIC
    variable = var
    function_x = 'x + y + z'
    function_y = 'x + y + z'
    function_z = 'x + y + z'
  []
[]

[Variables]
  [var]
    order = FIRST
    family = LAGRANGE_VEC
  []
[]

[AuxVariables]
  [old_var]
    order = FIRST
    family = LAGRANGE_VEC
  []
  [old_var_mag]
    order = FIRST
    family = LAGRANGE
  []
  [var_mag]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxKernels]
  [old]
    type = VectorCoupledOldAux
    variable = old_var
    v = 'var var'
    execute_on = TIMESTEP_END
  []

  [var_mag]
    type = VectorVariableMagnitudeAux
    variable = var_mag
    vector_variable = var
  []
  [old_var_mag]
    type = VectorVariableMagnitudeAux
    variable = old_var_mag
    vector_variable = old_var
  []
[]
[Executioner]
  type = Transient
  num_steps = 10
  dt = 0.1
[]

[Outputs]
  exodus = true
  csv = true
[]

[VectorPostprocessors]
  [var]
    type = LineValueSampler
    end_point = '1 1 1'
    num_points = 10
    sort_by = x
    start_point = '0 0 0'
    variable = var_mag
  []
  [old_var]
    type = LineValueSampler
    end_point = '1 1 1'
    num_points = 10
    sort_by = x
    start_point = '0 0 0'
    variable = old_var_mag
  []
[]
