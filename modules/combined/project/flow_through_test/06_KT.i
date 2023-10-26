#production_rate = 1.0 # kg/s/m
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 20
    ny = 10
    xmin = 0
    xmax = 0.0508 #m
    ymin = 0
    ymax = 0.0254 #m
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
  #gravity = '0 0 0'
[]

[Variables]
  [porepressure]
    #initial_condition = 1E6
  []
  [f_H]
    initial_condition = 8.201229858451E-07
  []
[]

[PorousFlowFullySaturated]
  porepressure = porepressure
  coupling_type = Hydro
  gravity = '0 0 0'
  fp = the_simple_fluid
  mass_fraction_vars = f_H
  stabilization = Full
  #flux_limiter_type = superbee
[]

[BCs]
  [constant_injection_porepressure]
    type = DirichletBC
    variable = porepressure
    value = 2E6
    boundary = left
  []
  [constant_outer_porepressure]
    type = DirichletBC
    variable = porepressure
    value = 1E0
    boundary = right
  []
  [injected_H]
    type = DirichletBC
    variable = f_H
    value = 0.1
    boundary = left
  []
[]

# [DiracKernels]
#   [produce_H]
#     type = PorousFlowPolyLineSink
#     variable = f_H
#     SumQuantityUO = produced_mass_H
#     mass_fraction_component = 0
#     point_file = production.bh
#     line_length = 0.0254
#     fluxes = ${production_rate}
#     p_or_t_vals = 0.0
#   []
# []

[UserObjects]
  [produced_mass_H]
    type = PorousFlowSumQuantity
  []
[]

[Postprocessors]
  [mass_extracted_H]
    type = PorousFlowPlotQuantity
    uo = produced_mass_H
    execute_on = 'initial timestep_end'
  []
  [dt]
    type = TimestepSize
    execute_on = 'timestep_begin'
  []
[]

[FluidProperties]
  [the_simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 2E9
    viscosity = 1.0E-3
    density0 = 1000.0
  []
[]

[Materials]
  [porosity]
    type = PorousFlowPorosity
    porosity_zero = 0.1
  []
  [permeability_aquifer]
    type = PorousFlowPermeabilityConst
    permeability = '1E-14 0 0   0 1E-14 0   0 0 1E-14'
  []
  # [permeability_caps]
  #   type = PorousFlowPermeabilityConst
  #   block = caps
  #   permeability = '1E-15 0 0   0 1E-15 0   0 0 1E-16'
  # []
[]

[Preconditioning]
  active = basic
  [basic]
    type = SMP
    full = true
    petsc_options = '-ksp_diagonal_scale -ksp_diagonal_scale_fix'
    petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap'
    petsc_options_value = ' asm      lu           NONZERO                   2'
  []
  [preferred]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
    petsc_options_value = ' lu       mumps'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 1E6
  dt = 1E2
  nl_rel_tol = 1E-14
[]

[Outputs]
  exodus = true
[]
