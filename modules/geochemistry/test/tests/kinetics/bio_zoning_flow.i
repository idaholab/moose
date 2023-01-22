# groundwater velocity is 10m.yr^-1 divided by porosity of 0.3
# The following are the mole numbers of the species in the groundwater
# The numerical values can be obtained by running the geochemistry simulation with a very small timestep so no kinetics are active (use the transported_bulk_moles values)
eqm_H2O = 55.49986252429319
eqm_CH3COO = 1e-9
eqm_CH4 = 1e-9
eqm_HS = 1e-9
eqm_Ca = 1e-3
eqm_SO4 = 4e-5
eqm_Fe = 1.386143651587732e-05
# The following are scalings used in calculating the residual.  Eg, because the concentration of CH3COO is so low, its residual is always tiny, so to get better accuracy it should be scaled
scale_H2O = ${fparse 1.0 / eqm_H2O}
scale_CH3COO = ${fparse 1.0 / eqm_CH3COO}
scale_CH4 = ${fparse 1.0 / eqm_CH4}
scale_HS = ${fparse 1.0 / eqm_HS}
scale_Ca = ${fparse 1.0 / eqm_Ca}
scale_SO4 = ${fparse 1.0 / eqm_SO4}
scale_Fe = ${fparse 1.0 / eqm_Fe}
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 500
    xmin = 0
    xmax = 200000
  []
[]

[UserObjects]
  [nodal_void_volume_uo]
    type = NodalVoidVolume
    porosity = 1.0
    execute_on = 'initial'
  []
[]

[Variables]
  [conc_H2O]
    initial_condition = ${eqm_H2O}
    scaling = ${scale_H2O}
  []
  [conc_CH3COO]
    initial_condition = ${eqm_CH3COO}
    scaling = ${scale_CH3COO}
  []
  [conc_CH4]
    initial_condition = ${eqm_CH4}
    scaling = ${scale_CH4}
  []
  [conc_HS]
    initial_condition = ${eqm_HS}
    scaling = ${scale_HS}
  []
  [conc_Ca]
    initial_condition = ${eqm_Ca}
    scaling = ${scale_Ca}
  []
  [conc_SO4]
    initial_condition = ${eqm_SO4}
    scaling = ${scale_SO4}
  []
  [conc_Fe]
    initial_condition = ${eqm_Fe}
    scaling = ${scale_Fe}
  []
[]

[Kernels]
  [dot_H2O]
    type = GeochemistryTimeDerivative
    variable = conc_H2O
    save_in = rate_H2O_times_vv
  []
  [dot_CH3COO]
    type = GeochemistryTimeDerivative
    variable = conc_CH3COO
    save_in = rate_CH3COO_times_vv
  []
  [dot_CH4]
    type = GeochemistryTimeDerivative
    variable = conc_CH4
    save_in = rate_CH4_times_vv
  []
  [dot_HS]
    type = GeochemistryTimeDerivative
    variable = conc_HS
    save_in = rate_HS_times_vv
  []
  [dot_Ca]
    type = GeochemistryTimeDerivative
    variable = conc_Ca
    save_in = rate_Ca_times_vv
  []
  [dot_SO4]
    type = GeochemistryTimeDerivative
    variable = conc_SO4
    save_in = rate_SO4_times_vv
  []
  [dot_Fe]
    type = GeochemistryTimeDerivative
    variable = conc_Fe
    save_in = rate_Fe_times_vv
  []
  [adv_H2O]
    type = ConservativeAdvection
    velocity = velocity
    upwinding_type = full
    variable = conc_H2O
  []
  [adv_CH3COO]
    type = ConservativeAdvection
    velocity = velocity
    upwinding_type = full
    variable = conc_CH3COO
  []
  [adv_CH4]
    type = ConservativeAdvection
    velocity = velocity
    upwinding_type = full
    variable = conc_CH4
  []
  [adv_HS]
    type = ConservativeAdvection
    velocity = velocity
    upwinding_type = full
    variable = conc_HS
  []
  [adv_Ca]
    type = ConservativeAdvection
    velocity = velocity
    upwinding_type = full
    variable = conc_Ca
  []
  [adv_SO4]
    type = ConservativeAdvection
    velocity = velocity
    upwinding_type = full
    variable = conc_SO4
  []
  [adv_Fe]
    type = ConservativeAdvection
    velocity = velocity
    upwinding_type = full
    variable = conc_Fe
  []
[]

[AuxVariables]
  [velocity]
    family = MONOMIAL_VEC
    order = CONSTANT
  []
  [nodal_void_volume]
  []
  [rate_H2O_times_vv]
  []
  [rate_CH3COO_times_vv]
  []
  [rate_CH4_times_vv]
  []
  [rate_HS_times_vv]
  []
  [rate_Ca_times_vv]
  []
  [rate_SO4_times_vv]
  []
  [rate_Fe_times_vv]
  []
  [rate_H2O]
  []
  [rate_CH3COO]
  []
  [rate_CH4]
  []
  [rate_HS]
  []
  [rate_Ca]
  []
  [rate_SO4]
  []
  [rate_Fe]
  []
[]

[AuxKernels]
  [velocity]
    type = VectorFunctionAux
    function = vel_fcn
    variable = velocity
  []
  [nodal_void_volume_auxk]
    type = NodalVoidVolumeAux
    variable = nodal_void_volume
    nodal_void_volume_uo = nodal_void_volume_uo
    execute_on = 'initial timestep_end' # "initial" to ensure it is properly evaluated for the first timestep
  []
  [rate_H2O_auxk]
    type = ParsedAux
    variable = rate_H2O
    args = 'rate_H2O_times_vv nodal_void_volume'
    function = 'rate_H2O_times_vv / nodal_void_volume'
  []
  [rate_CH3COO]
    type = ParsedAux
    variable = rate_CH3COO
    args = 'rate_CH3COO_times_vv nodal_void_volume'
    function = 'rate_CH3COO_times_vv / nodal_void_volume'
  []
  [rate_CH4]
    type = ParsedAux
    variable = rate_CH4
    args = 'rate_CH4_times_vv nodal_void_volume'
    function = 'rate_CH4_times_vv / nodal_void_volume'
  []
  [rate_HS]
    type = ParsedAux
    variable = rate_HS
    args = 'rate_HS_times_vv nodal_void_volume'
    function = 'rate_HS_times_vv / nodal_void_volume'
  []
  [rate_Ca]
    type = ParsedAux
    variable = rate_Ca
    args = 'rate_Ca_times_vv nodal_void_volume'
    function = 'rate_Ca_times_vv / nodal_void_volume'
  []
  [rate_SO4]
    type = ParsedAux
    variable = rate_SO4
    args = 'rate_SO4_times_vv nodal_void_volume'
    function = 'rate_SO4_times_vv / nodal_void_volume'
  []
  [rate_Fe]
    type = ParsedAux
    variable = rate_Fe
    args = 'rate_Fe_times_vv nodal_void_volume'
    function = 'rate_Fe_times_vv / nodal_void_volume'
  []
[]

[Functions]
  [vel_fcn]
    type = ParsedVectorFunction
    expression_x = 33.333333
    expression_y = 0
    expression_z = 0
  []
[]

[BCs]
  [inject_H2O]
    type = DirichletBC
    boundary = 'left right'
    variable = conc_H2O
    value = ${eqm_H2O}
  []
  [inject_CH3COO]
    type = DirichletBC
    boundary = 'left right'
    variable = conc_CH3COO
    value = ${eqm_CH3COO}
  []
  [inject_CH4]
    type = DirichletBC
    boundary = 'left right'
    variable = conc_CH4
    value = ${eqm_CH4}
  []
  [inject_HS]
    type = DirichletBC
    boundary = 'left right'
    variable = conc_HS
    value = ${eqm_HS}
  []
  [inject_Ca]
    type = DirichletBC
    boundary = 'left right'
    variable = conc_Ca
    value = ${eqm_Ca}
  []
  [inject_SO4]
    type = DirichletBC
    boundary = 'left right'
    variable = conc_SO4
    value = ${eqm_SO4}
  []
[]

[Preconditioning]
  [typically_efficient]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_hypre_type'
    petsc_options_value = ' hypre    boomeramg'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  [TimeStepper]
    type = FunctionDT
  function = 'min(0.1 * (t + 1), 100)'
  []
  end_time = 20000
  nl_abs_tol = 1E-5
[]

[Outputs]
  csv = true
[]

[MultiApps]
  [react]
    type = TransientMultiApp
    input_files = bio_zoning_conc.i
    clone_parent_mesh = true
    execute_on = 'timestep_end' # This is critical
  []
[]

[Transfers]
  [changes_due_to_flow]
    type = MultiAppCopyTransfer
    to_multi_app = react
    source_variable = 'rate_H2O rate_CH3COO rate_CH4 rate_HS rate_Ca rate_SO4 rate_Fe' # change in mole number at every node / dt
    variable = 'rate_H2O_per_1l rate_CH3COO_per_1l rate_CH4_per_1l rate_HS_per_1l rate_Ca_per_1l rate_SO4_per_1l rate_Fe_per_1l' # change in moles at every node / dt
  []
  [transported_moles_from_geochem]
    type = MultiAppCopyTransfer
    from_multi_app = react
    source_variable = 'transported_H2O transported_CH3COO transported_CH4 transported_HS transported_Ca transported_SO4 transported_Fe'
    variable = 'conc_H2O conc_CH3COO conc_CH4 conc_HS conc_Ca conc_SO4 conc_Fe'
  []
[]

