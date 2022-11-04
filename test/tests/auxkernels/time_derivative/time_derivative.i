# This test is setting the values of an auxiliary varaible f according to the
# function f_fn. This function is time dependent.
#
# Then the f_dot is brought as a forcing function into the L2 projection, thus
# the resulting values of u should give the f_dot which is known.
#
# NOTE: There is no need to do more than 2 time steps, because f_dot is constant
# in time. That means that the projection is exactly the same for the second time
# step as is for the first time step. The solver has nothing to do and you can
# see that on the "zero" initial non-linear residual.
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 2
  ny = 2
[]

[Functions]
  [./f_fn]
    type = ParsedFunction
    expression = t*(x+y)
  [../]
  [./f_dot_fn]
    type = ParsedFunction
    expression = (x+y)
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./l2_proj]
    type = Reaction
    variable = u
  [../]
  [./dck]
    type = DotCouplingKernel
    variable = u
    v = f
  [../]
[]

[AuxVariables]
  [./f]
  [../]
[]

[AuxKernels]
  [./f_k]
    type = FunctionAux
    variable = f
    function = f_fn
  [../]
[]

[Postprocessors]
  [./l2_error]
    type = ElementL2Error
    variable = u
    function = f_dot_fn
  [../]
[]

[Executioner]
  type = Transient

  dt = 0.1
  num_steps = 2

  nl_abs_tol = 1.e-15
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
