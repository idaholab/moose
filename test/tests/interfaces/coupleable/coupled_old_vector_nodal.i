# Test for coupledVectorValuesOld for nodal variables

[Mesh]
  [gmg]
    type=GeneratedMeshGenerator
    dim=3
    nx = 10
    ny = 1
    nz = 1
  []
[]

[Kernels]
  [time_deriv]
    type=VectorTimeDerivative
    variable=var
  []
  [bodyf]
    type=VectorBodyForce
    variable=var
    function_x='-1'
    function_y='-1'
    function_z='-1'
  []
[]


[ICs]
  [ics]
    type=VectorFunctionIC
    variable=var
    function_x='x + y + z'
    function_y='x + y + z'
    function_z='x + y + z'
  []
[]

[Variables]
  [var]
    order=FIRST
    family =LAGRANGE_VEC
  []
[]

[AuxVariables]
  [old_var]
    order=FIRST
    family =LAGRANGE_VEC
  []
[]

[AuxKernels]
  [old]
    type=VectorCoupledOldAux
    variable=old_var
    v='var var'
    execute_on=TIMESTEP_END
  []

[]
[Executioner]
  type=Transient
  num_steps=10
  dt=0.1
[]

[Outputs]
  exodus=true
[]
