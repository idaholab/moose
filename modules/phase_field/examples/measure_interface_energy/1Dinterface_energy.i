[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 100
  xmax = 100
  xmin = 0
  elem_type = EDGE
[]

[AuxVariables]
  [./local_energy]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./local_free_energy]
    type = TotalFreeEnergy
    variable = local_energy
    kappa_names = kappa_c
    interfacial_vars = c
  [../]
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
    scaling = 1e1
    [./InitialCondition]
      type = RampIC
      variable = c
      value_left = 0
      value_right = 1
    [../]
  [../]
  [./w]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./c_res]
    type = SplitCHParsed
    variable = c
    f_name = F
    kappa_name = kappa_c
    w = w
  [../]
  [./w_res]
    type = SplitCHWRes
    variable = w
    mob_name = M
  [../]
  [./time]
    type = CoupledTimeDerivative
    variable = w
    v = c
  [../]

[]

[Functions]
  [./Int_energy]
    type = ParsedFunction
    symbol_values = 'total_solute Cleft Cright Fleft Fright volume'
    expression = '((total_solute-Cleft*volume)/(Cright-Cleft))*Fright+(volume-(total_solute-Cleft*volume)/(Cright-Cleft))*Fleft'
    symbol_names = 'total_solute Cleft Cright Fleft Fright volume'
  [../]
  [./Diff]
    type = ParsedFunction
    symbol_values = 'total_free_energy total_no_int'
    symbol_names = 'total_free_energy total_no_int'
    expression = total_free_energy-total_no_int
  [../]
[]

[Materials]
  [./consts]
    type = GenericConstantMaterial
    prop_names  = 'kappa_c M'
    prop_values = '25      150'
  [../]
  [./Free_energy]
    type = DerivativeParsedMaterial
    property_name = F
    expression = 'c^2*(c-1)^2'
    coupled_variables = c
    derivative_order = 2
  [../]
[]

[Postprocessors]
  # The total free energy of the simulation cell to observe the energy reduction.
  [./total_free_energy]
    type = ElementIntegralVariablePostprocessor
    variable = local_energy
  [../]

  # for testing we also monitor the total solute amount, which should be conserved,
  # gives Cavg in % for this problem.
  [./total_solute]
    type = ElementIntegralVariablePostprocessor
    variable = c
  [../]
  # Get simulation cell size (1D volume) from postprocessor
  [./volume]
    type = ElementIntegralMaterialProperty
    mat_prop = 1
  [../]
  # Find concentration in each phase using SideAverageValue
  [./Cleft]
    type = SideAverageValue
    boundary = left
    variable = c
  [../]
  [./Cright]
    type = SideAverageValue
    boundary = right
    variable = c
  [../]
  # Find local energy in each phase by checking boundaries
  [./Fleft]
    type = SideAverageValue
    boundary = left
    variable = local_energy
  [../]
  [./Fright]
    type = SideAverageValue
    boundary = right
    variable = local_energy
  [../]
  # Use concentrations and energies to find total free energy without any interface,
  # only applies once equilibrium is reached!!
  # Difference between energy with and without interface
  # gives interface energy per unit area.
  [./total_no_int]
    type = FunctionValuePostprocessor
    function = Int_energy
  [../]
  [./Energy_of_Interface]
    type = FunctionValuePostprocessor
    function = Diff
  [../]
[]

[Preconditioning]
  # This preconditioner makes sure the Jacobian Matrix is fully populated. Our
  # kernels compute all Jacobian matrix entries.
  # This allows us to use the Newton solver below.
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  # Automatic differentiation provides a _full_ Jacobian in this example
  # so we can safely use NEWTON for a fast solve
  solve_type = 'NEWTON'

  l_max_its = 15
  l_tol = 1.0e-6

  nl_max_its = 15
  nl_rel_tol = 1.0e-10
  nl_abs_tol = 1.0e-4

  start_time = 0.0
  # make sure that the result obtained for the interfacial free energy is fully converged
  end_time   = 40

  [./TimeStepper]
    type = SolutionTimeAdaptiveDT
    dt = 0.5
  [../]
[]

[Outputs]
  gnuplot = true
  csv = true
  [./exodus]
    type = Exodus
    show = 'c local_energy'
    execute_on = 'failed initial nonlinear timestep_end final'
  [../]
  [./console]
    type = Console
    execute_on = 'FAILED INITIAL NONLINEAR TIMESTEP_END final'
  [../]
  perf_graph = true
[]
