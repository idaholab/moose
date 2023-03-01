#
# Test the DiscreteNucleation material in a toy system. The global
# concentration is above the solubility limit, but below the spinodal.
# Without further intervention no nucleation will occur in a phase
# field model. The DiscreteNucleation material will locally modify the
# free energy to coerce nuclei to grow.
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 120
  ny = 120
  xmax = 500
  ymax = 500
  elem_type = QUAD
[]

[Modules]
  [./PhaseField]
    [./Conserved]
      [./c]
        free_energy = F
        mobility = M
        kappa = kappa_c
        solve_type = REVERSE_SPLIT
      [../]
    [../]
  [../]
[]

[ICs]
  [./c_IC]
    type = RandomIC
    variable = c
    min = 0.2
    max = 0.21
  [../]
[]

[Materials]
  [./pfmobility]
    type = GenericConstantMaterial
    prop_names  = 'M kappa_c'
    prop_values = '1 25'
  [../]
  [./chemical_free_energy]
    # simple double well free energy
    type = DerivativeParsedMaterial
    property_name = Fc
    coupled_variables = 'c'
    constant_names       = 'barr_height  cv_eq'
    constant_expressions = '0.1          0'
    expression = 16*barr_height*c^2*(1-c)^2 # +0.01*(c*plog(c,0.005)+(1-c)*plog(1-c,0.005))
    derivative_order = 2
    outputs = exodus
  [../]
  [./probability]
    # This is a made up toy nucleation rate it should be replaced by
    # classical nucleation theory in a real simulation.
    type = ParsedMaterial
    property_name = P
    coupled_variables = c
    expression = c*1e-7
    outputs = exodus
  [../]
  [./nucleation]
    # The nucleation material is configured to insert nuclei into the free energy
    # tht force the concentration to go to 0.95, and holds this enforcement for 500
    # time units.
    type = DiscreteNucleation
    property_name = Fn
    op_names  = c
    op_values = 0.90
    penalty = 5
    penalty_mode = MIN
    map = map
    outputs = exodus
  [../]
  [./free_energy]
    # add the chemical and nucleation free energy contributions together
    type = DerivativeSumMaterial
    derivative_order = 2
    coupled_variables = c
    sum_materials = 'Fc Fn'
  [../]
[]

[UserObjects]
  [./inserter]
    # The inserter runs at the end of each time step to add nucleation events
    # that happend during the timestep (if it converged) to the list of nuclei
    type = DiscreteNucleationInserter
    hold_time = 100
    probability = P
    radius = 10
  [../]
  [./map]
    # The map UO runs at the beginning of a timestep and generates a per-element/qp
    # map of nucleus locations. The map is only regenerated if the mesh changed or
    # the list of nuclei was modified.
    # The map converts the nucleation points into finite area objects with a given radius.
    type = DiscreteNucleationMap
    periodic = c
    inserter = inserter
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      auto_direction = 'x y'
    [../]
  [../]
[]

[Postprocessors]
  [./dt]
    type = TimestepSize
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -sub_pc_type'
  petsc_options_value = 'asm      lu          '

  nl_max_its = 20
  l_tol = 1.0e-4
  nl_rel_tol = 1.0e-10
  nl_abs_tol = 1.0e-10
  start_time = 0.0
  num_steps = 1200

  [./TimeStepper]
    type = IterationAdaptiveDT
    dt = 10
    growth_factor = 1.5
    cutback_factor = 0.5
    optimal_iterations = 5
  [../]
[]

[Outputs]
  exodus = true
[]
