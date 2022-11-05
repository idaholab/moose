#
# Simulation of iron-chromium alloy decomposition using simplified conditions.
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  elem_type = QUAD4
  nx = 25
  ny = 25
  nz = 0
  xmin = 0
  xmax = 25
  ymin = 0
  ymax = 25
  zmin = 0
  zmax = 0
  uniform_refine = 2
[]

[Variables]
  [./c]   # Mole fraction of Cr (unitless)
    order = FIRST
    family = LAGRANGE
  [../]
  [./w]   # Chemical potential (eV/mol)
    order = FIRST
    family = LAGRANGE
  [../]
[]

[ICs]
  [./concentrationIC]   # 46.774 mol% Cr with variations
    type = RandomIC
    min = 0.44774
    max = 0.48774
    seed = 210
    variable = c
  [../]
[]

[BCs]
  [./Periodic]
    [./c_bcs]
      auto_direction = 'x y'
    [../]
  [../]
[]

[Kernels]
  [./w_dot]
    variable = w
    v = c
    type = CoupledTimeDerivative
  [../]
  [./coupled_res]
    variable = w
    type = SplitCHWRes
    mob_name = M
  [../]
  [./coupled_parsed]
    variable = c
    type = SplitCHParsed
    f_name = f_loc
    kappa_name = kappa_c
    w = w
  [../]
[]

[Materials]
  # d is a scaling factor that makes it easier for the solution to converge
  # without changing the results. It is defined in each of the materials and
  # must have the same value in each one.
  [./constants]
    # Define constant values kappa_c and M. Eventually M will be replaced with
    # an equation rather than a constant.
    type = GenericFunctionMaterial
    prop_names = 'kappa_c M'
    prop_values = '8.125e-16*6.24150934e+18*1e+09^2*1e-27
                   2.2841e-26*1e+09^2/6.24150934e+18/1e-27'
                   # kappa_c*eV_J*nm_m^2*d
                   # M*nm_m^2/eV_J/d
  [../]
  [./local_energy]
    # Defines the function for the local free energy density as given in the
    # problem, then converts units and adds scaling factor.
    type = DerivativeParsedMaterial
    property_name = f_loc
    coupled_variables = c
    constant_names = 'A   B   C   D   E   F   G  eV_J  d'
    constant_expressions = '-2.446831e+04 -2.827533e+04 4.167994e+03 7.052907e+03
                            1.208993e+04 2.568625e+03 -2.354293e+03
                            6.24150934e+18 1e-27'
    expression = 'eV_J*d*(A*c+B*(1-c)+C*c*log(c)+D*(1-c)*log(1-c)+
                E*c*(1-c)+F*c*(1-c)*(2*c-1)+G*c*(1-c)*(2*c-1)^2)'
    derivative_order = 2
  [../]
[]

[Postprocessors]
  [./step_size]             # Size of the time step
    type = TimestepSize
  [../]
  [./iterations]            # Number of iterations needed to converge timestep
    type = NumNonlinearIterations
  [../]
  [./nodes]                 # Number of nodes in mesh
    type = NumNodes
  [../]
  [./evaluations]           # Cumulative residual calculations for simulation
    type = NumResidualEvaluations
  [../]
  [./active_time]           # Time computer spent on simulation
    type = PerfGraphData
    section_name = "Root"
    data_type = total
  [../]
[]

[Preconditioning]
  [./coupled]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  l_max_its = 30
  l_tol = 1e-6
  nl_max_its = 50
  nl_abs_tol = 1e-9
  end_time = 604800   # 7 days
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_ksp_type
                         -sub_pc_type -pc_asm_overlap'
  petsc_options_value = 'asm      31                  preonly
                         ilu          1'
  [./TimeStepper]
    type = IterationAdaptiveDT
    dt = 10
    cutback_factor = 0.8
    growth_factor = 1.5
    optimal_iterations = 7
  [../]
  [./Adaptivity]
    coarsen_fraction = 0.1
    refine_fraction = 0.7
    max_h_level = 2
  [../]
[]

[Debug]
  show_var_residual_norms = true
[]

[Outputs]
  exodus = true
  console = true
  csv = true
  [./console]
    type = Console
    max_rows = 10
  [../]
[]
