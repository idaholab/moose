mu=1
rho=1
k=1e-3
cp=1

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 5
    ymin = -1
    ymax = 1
    nx = 10
    ny = 4
  []
  [right]
    type = ParsedSubdomainMeshGenerator
    input = gen
    combinatorial_geometry = 'x > 2.5'
    block_id = 1
  []
  [left]
    type = ParsedSubdomainMeshGenerator
    input = right
    combinatorial_geometry = 'x < 2.5'
    block_id = 2
  []
[]

[Modules]
  [NavierStokesFV]
    compressibility = 'incompressible'
    add_energy_equation = true

    density = ${rho}
    dynamic_viscosity = ${mu}
    thermal_conductivity = ${k}
    specific_heat = ${cp}

    inlet_boundaries = 'left'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = '1 0'
    energy_inlet_types = 'fixed-temperature'
    energy_inlet_function = '1'

    wall_boundaries = 'top bottom'
    momentum_wall_types = 'noslip noslip'
    energy_wall_types = 'heatflux heatflux'
    energy_wall_function = '0 0'

    outlet_boundaries = 'right'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = '0'
  []
[]

[Materials]
  [kappa]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'kappa'
    prop_values = '1 1 1'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      100                lu           NONZERO'
  line_search = 'none'
  nl_rel_tol = 1e-12
[]
