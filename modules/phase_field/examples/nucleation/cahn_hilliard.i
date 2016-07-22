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
  xmin = 0
  xmax = 500
  ymin = 0
  ymax = 500
  elem_type = QUAD4
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = RandomIC
      min = 0.2
      max = 0.21
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

[Materials]
  [./pfmobility]
    type = GenericConstantMaterial
    prop_names  = 'M kappa_c'
    prop_values = '1 25'
  [../]

  [./chemical_free_energy]
    # simple double well free energy
    type = DerivativeParsedMaterial
    f_name = Fc
    args = 'c'
    constant_names       = 'barr_height  cv_eq'
    constant_expressions = '0.1          0'
    function = 16*barr_height*c^2*(1-c)^2 # +0.01*(c*plog(c,0.005)+(1-c)*plog(1-c,0.005))
    derivative_order = 2
    outputs = exodus
  [../]
  [./probability]
    # This is a made up toy nucleation rate it should be replaced by
    # classical nucleation theory in a real simulation.
    type = ParsedMaterial
    f_name = P
    args = c
    function = c*1e-7
    outputs = exodus
  [../]
  [./nucleation]
    # The nucleation material is configured to insert nuclei into the free energy
    # tht force the concentration to go to 0.95, and holds this enforcement for 500
    # time units.
    type = DiscreteNucleation
    f_name = Fn
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
    args = c
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
  [../]
  [./map]
    # The map UO runs at the beginning of a timestep and generates a per-element/qp
    # map of nucleus locations. The map is only regenerated if the mesh changed or
    # the list of nuclei was modified.
    # The map converts the nucleation points into finite area objects with a given radius.
    type = DiscreteNucleationMap
    radius = 10
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

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = 'NEWTON'

  nl_max_its = 15
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
  [../]
[]

[Outputs]
  exodus = true
[]
