# Test for coupledVectorValuesOld for constant monomials

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
    function_y='2*(x + y + z)'
    function_z='3*(x + y + z)'
  []
[]

[Variables]
  [var]
    order=CONSTANT
    family =MONOMIAL_VEC
  []
[]

[AuxVariables]
  [old_var]
    order=CONSTANT
    family =MONOMIAL_VEC
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
