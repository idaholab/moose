[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  nz = 0
  xmin = 0
  xmax = 500
  ymin = 0
  ymax = 500
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[GlobalParams]
  op_num = 4
  var_name_base = gr
[]

[Variables]
  [PolycrystalVariables]
  []
[]

[Functions]
  [grain_growth_energy]
    type = PiecewiseMultilinear
    data_file = grain_growth_energy.data
  []
  [grain_growth_mu0]
    type = PiecewiseMultilinear
    data_file = grain_growth_mu0.data
  []
  [grain_growth_mu1]
    type = PiecewiseMultilinear
    data_file = grain_growth_mu1.data
  []
  [grain_growth_mu2]
    type = PiecewiseMultilinear
    data_file = grain_growth_mu2.data
  []
  [grain_growth_mu3]
    type = PiecewiseMultilinear
    data_file = grain_growth_mu3.data
  []

  [matrix]
    type = ParsedFunction
    expression = '1-x-y-z'
  []
[]

[ICs]
  [gr1]
    type = SmoothCircleIC
    variable = gr1
    x1 = 0
    y1 = 0
    radius = 150
    int_width = 90
    invalue = 1
    outvalue = 0
  []
  [gr2]
    type = SmoothCircleIC
    variable = gr2
    x1 = 500
    y1 = 0
    radius = 120
    int_width = 90
    invalue = 1
    outvalue = 0
  []
  [gr3]
    type = SmoothCircleIC
    variable = gr3
    x1 = 250
    y1 = 500
    radius = 300
    int_width = 90
    invalue = 1
    outvalue = 0
  []
  [gr0]
    type = CoupledValueFunctionIC
    variable = gr0
    v = 'gr1 gr2 gr3'
    function = matrix
  []
[]

[AuxVariables]
  [bnds]
    order = FIRST
    family = LAGRANGE
  []
  [local_energy]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Kernels]
  [gr0dot]
    type = TimeDerivative
    variable = gr0
  []
  [gr0bulk]
    type = AllenCahn
    variable = gr0
    f_name = F
    coupled_variables = 'gr1 gr2 gr3'
  []
  [gr0int]
    type = ACInterface
    variable = gr0
    kappa_name = kappa_op
  []

  [gr1dot]
    type = TimeDerivative
    variable = gr1
  []
  [gr1bulk]
    type = AllenCahn
    variable = gr1
    f_name = F
    coupled_variables = 'gr0 gr2 gr3'
  []
  [gr1int]
    type = ACInterface
    variable = gr1
    kappa_name = kappa_op
  []

  [gr2dot]
    type = TimeDerivative
    variable = gr2
  []
  [gr2bulk]
    type = AllenCahn
    variable = gr2
    f_name = F
    coupled_variables = 'gr0 gr1 gr3'
  []
  [gr2int]
    type = ACInterface
    variable = gr2
    kappa_name = kappa_op
  []

  [gr3dot]
    type = TimeDerivative
    variable = gr3
  []
  [gr3bulk]
    type = AllenCahn
    variable = gr3
    f_name = F
    coupled_variables = 'gr0 gr1 gr2'
  []
  [gr3int]
    type = ACInterface
    variable = gr3
    kappa_name = kappa_op
  []
[]

[AuxKernels]
  [BndsCalc]
    type = BndsCalcAux
    variable = bnds
  []
  [local_free_energy]
    type = TotalFreeEnergy
    variable = local_energy
    kappa_names = 'kappa_op kappa_op kappa_op kappa_op'
    interfacial_vars = 'gr0 gr1 gr2 gr3'
  []
[]

[Materials]
  [Copper]
    type = GBEvolution
    T = 500 # K
    wGB = 60 # nm
    GBmob0 = 2.5e-6 # m^4/(Js) from Schoenfelder 1997
    Q = 0.23 # Migration energy in eV
    GBenergy = 0.708 # GB energy in J/m^2
  []
  [Tabulated]
    type = CoupledValueFunctionFreeEnergy
    free_energy_function = grain_growth_energy
    chemical_potential_functions = 'grain_growth_mu0 grain_growth_mu1 grain_growth_mu2 '
                                   'grain_growth_mu3'
    v = 'gr0 gr1 gr2 gr3'
  []
[]

[Postprocessors]
  [total_energy]
    type = ElementIntegralVariablePostprocessor
    variable = local_energy
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    coupled_groups = 'gr0,gr1 gr0,gr2 gr0,gr3'
  []
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = NEWTON
  l_tol = 1.0e-4
  l_max_its = 30
  nl_max_its = 30
  nl_rel_tol = 1.0e-9
  start_time = 0.0
  num_steps = 3
  dt = 100.0
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
  perf_graph = true
[]
