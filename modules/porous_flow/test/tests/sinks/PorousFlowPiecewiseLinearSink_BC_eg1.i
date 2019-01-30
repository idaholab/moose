## This is an example input file showing how to set a Type I BC with PorousFlowPiecewiseLinearSink
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  xmin = 0
  xmax = 1
  ny = 10
  ymin = 0
  ymax = 1
[]

[MeshModifiers]
  [./aquifer]
    type = SubdomainBoundingBox
    block_id = 11
    bottom_left = '0 0 0'
    top_right = '1 1 1'
  [../]
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [./porepressure]
    initial_condition = 1.e6 # initial pressure in domain
  [../]
[]

[PorousFlowBasicTHM]
  porepressure = porepressure
  coupling_type = Hydro
  gravity = '0 0 0'
  fp = the_simple_fluid
  number_fluid_phases = 1
  number_fluid_components = 1
[]

[AuxVariables]
  [./fluxes_out]
  [../]
[]

[BCs]
  [./in_left]
    type = PorousFlowPiecewiseLinearSink
    variable = porepressure
    boundary = 'left'
    pt_vals = '-1e9 1e9' # x coordinates defining g
    multipliers = '-1e9 1e9' # y coordinates defining g
    PT_shift = 2.E6   # BC pressure
    flux_function = 1E-6 # Variable C
    fluid_phase = 0
    save_in = fluxes_out
  [../]
[]

[Postprocessors]
  [./left_flux]
    type = NodalSum
    boundary = 'left'
    variable = fluxes_out
    execute_on = 'timestep_end'
  [../]
[]

[Modules]
  [./FluidProperties]
    [./the_simple_fluid]
      type = SimpleFluidProperties
      bulk_modulus = 2E9
      viscosity = 1.0E-3
      density0 = 1000.0
    [../]
  [../]
[]

[Materials]
  [./porosity]
    type = PorousFlowPorosity
    porosity_zero = 0.1
  [../]
  [./biot_modulus]
    type = PorousFlowConstantBiotModulus
    biot_coefficient = 0.8
    solid_bulk_compliance = 2E-7
    fluid_bulk_modulus = 1E7
  [../]
  [./permeability_aquifer]
    type = PorousFlowPermeabilityConst
    block = 11
    permeability = '1E-15 0 0   0 1E-15 0   0 0 1E-15'
  [../]
  #### The following Material give porepressure at nodes, which is required for PorousFlowPiecewiseLienarSink
  [./PS]
    type = PorousFlow1PhaseFullySaturated
    at_nodes = true
    porepressure = porepressure
  [../]
[]

[Preconditioning]
  active = basic
  [./basic]
    type = SMP
    full = true
    petsc_options = '-ksp_diagonal_scale -ksp_diagonal_scale_fix'
    petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap'
    petsc_options_value = ' asm      lu           NONZERO                   2'
  [../]
  [./preferred_but_might_not_be_installed]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
    petsc_options_value = ' lu       mumps'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 1E5
  dt = 1E3
  nl_abs_tol = 1E-10
[]

[Outputs]
  exodus = true
[]
