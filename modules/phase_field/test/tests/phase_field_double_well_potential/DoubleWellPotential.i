###########################################################
# This is a simple test of the PhaseFieldCoupledDoubleWellPotential System.
# It solves the a simple ODE du/dt = u*(u^2-1).
# Start time = 0.1
# u(0.1) = 0.9
# The simulation value is compared to the theoretical solution specified in the #function kernel
###########################################################

[Mesh]
  [linear]
    type = GeneratedMeshGenerator
    nx = 1
    dim = 1
  []
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[ICs]
  [ic]
   type = ConstantIC
   variable = u
   value = 0.9
  []
[]

[Kernels]
  [TimeDerivative]
    type = ADTimeDerivative
    variable = u
  []
  [bf]
    type = PhaseFieldCoupledDoubleWellPotential
    c = u
    variable = 'u'
    prefactor = -1.0
  []
[]

[Functions]
 [soln]
   type = ParsedFunction
   expression = '(1.0/(1.0 + e^(2*(t-0.825))))^0.5'
 []
[]
[Postprocessors]
 [simu_value]
  type = ElementalVariableValue
  variable = u
  elementid = 0
 []
 [theory_value]
   type = FunctionValuePostprocessor
   function = soln
 []
  [error]
    type = ElementL2Error
    function = soln
    variable = u
  []
[]

[Executioner]
  type = Transient 
  start_time = 0.1
  end_time = 0.102
  dt = 1e-4
  solve_type ='NEWTON'
[]

[Outputs]
 [csv]
   type = CSV
   time_step_interval = 1
 [] 
[]

