gravity_vector = '0 0 0'
initial_p = 1e5
initial_T = 300

porosity = 0.8
perm = 1e-8 # permeability
rho_matrix = 1000.0
cp_matrix = 500.0
k_matrix = 5.0

inj_point1 = '0.5 0.5 0.5'
pro_point1 = '3.5 1.5 1.5'

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = ${gravity_vector}
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  xmax = 4.0
  ymax = 3.0
  zmax = 2.0
  nx = 4
  ny = 3
  nz = 2
[]

[PorousFlowFullySaturated]
  coupling_type = ThermoHydro
  porepressure = porepressure
  temperature = temperature
  fp = fp
  pressure_unit = Pa
  stabilization = full
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
  []
[]

[Variables]
  [porepressure]
    initial_condition = ${initial_p}
  []
  [temperature]
    initial_condition = ${initial_T}
    scaling = 1e-6
  []
[]

[Materials]
  [porosity_mat]
    type = PorousFlowPorosityConst
    porosity = ${porosity}
  []
  [permeability_mat]
    type = PorousFlowPermeabilityConst
    permeability = '${perm} ${perm} ${perm} ${perm} ${perm} ${perm} ${perm} ${perm} ${perm}'
  []
  [matrix_energy_mat]
    type = PorousFlowMatrixInternalEnergy
    density = ${rho_matrix}
    specific_heat_capacity = ${cp_matrix}
  []
  [thermal_conductivity_mat]
    type = PorousFlowThermalConductivityIdeal
    dry_thermal_conductivity = '${k_matrix} 0 0 0 ${k_matrix} 0 0 0 ${k_matrix}'
  []
[]

[Executioner]
  type = Transient

  end_time = 5.0
  dt = 1.0

  solve_type = NEWTON
  line_search = none
  nl_max_its = 20
  nl_abs_tol = 1e-5
  nl_rel_tol = 1e-5
  l_tol = 1e-3
  l_max_its = 10
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  automatic_scaling = true
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    app_type = PorousFlowApp
    input_files = sub.i
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Physics]
  [CoupledInjectionProduction]
    [inj_prod]
      injection_points = '${inj_point1}'
      production_points = '${pro_point1}'
      multi_app = sub
    []
  []
[]

[Outputs]
  exodus = true
[]
