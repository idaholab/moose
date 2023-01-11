[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 1
  ny = 1
  elem_type = QUAD4
[]

[Variables]
  [./n]
    family = SCALAR
    order = FIRST
    initial_condition = 1
  [../]
[]

[ScalarKernels]
  [./dn]
    type = ODETimeDerivative
    variable = n
  [../]
  [./ode1]
    type = ParsedODEKernel
    expression = '-n'
    variable = n
    # implicit = false
  [../]
[]

[Executioner]
  type = Transient
  [./TimeIntegrator]
    # type = ImplicitEuler
    # type = BDF2
    type = CrankNicolson
    # type = ImplicitMidpoint
    # type = LStableDirk2
    # type = LStableDirk3
    # type = LStableDirk4
    # type = AStableDirk4
    #
    # Explicit methods
    # type = ExplicitEuler
    # type = ExplicitMidpoint
    # type = Heun
    # type = Ralston
  [../]
  start_time = 0
  end_time = 1
  dt = 0.001
  dtmin = 0.001 # Don't allow timestep cutting
  solve_type = 'PJFNK'
  nl_max_its = 2
  nl_abs_tol = 1.e-12 # This is an ODE, so nl_abs_tol makes sense.
[]

[Functions]
  [./exact_solution]
    type = ParsedFunction
    expression = exp(t)
  [../]
[]

[Postprocessors]
  [./error_n]
    # Post processor that computes the difference between the computed
    # and exact solutions.  For the exact solution used here, the
    # error at the final time should converge at O(dt^p), where p is
    # the order of the method.
    type = ScalarL2Error
    variable = n
    function = exact_solution
    # final is not currently supported for Postprocessor execute_on...
    # execute_on = 'final'
  [../]
[]

[Outputs]
  csv = true
[]
