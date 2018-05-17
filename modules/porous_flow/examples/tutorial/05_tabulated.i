# Darcy flow with heat advection and conduction, using Water97 properties
[Mesh]
  type = AnnularMesh
  dim = 2
  nr = 10
  rmin = 1.0
  rmax = 10
  growth_r = 1.4
  nt = 4
  tmin = 0
  tmax = 1.57079632679
[]

[MeshModifiers]
  [./make3D]
    type = MeshExtruder
    extrusion_vector = '0 0 12'
    num_layers = 3
    bottom_sideset = 'bottom'
    top_sideset = 'top'
  [../]
  [./shift_down]
    type = Transform
    transform = TRANSLATE
    vector_value = '0 0 -6'
    depends_on = make3D
  [../]
  [./aquifer]
    type = SubdomainBoundingBox
    block_id = 1
    bottom_left = '0 0 -2'
    top_right = '10 10 2'
    depends_on = shift_down
  [../]
  [./injection_area]
    type = ParsedAddSideset
    combinatorial_geometry = 'x*x+y*y<1.01'
    included_subdomain_ids = 1
    new_sideset_name = 'injection_area'
    depends_on = 'aquifer'
  [../]
  [./rename]
    type = RenameBlock
    old_block_id = '0 1'
    new_block_name = 'caps aquifer'
    depends_on = 'injection_area'
  [../]
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [./porepressure]
    initial_condition = 1E6
  [../]
  [./temperature]
    initial_condition = 313
    scaling = 1E-8
  [../]
[]

[PorousFlowBasicTHM]
  porepressure = porepressure
  temperature = temperature
  coupling_type = ThermoHydro
  gravity = '0 0 0'
  fp = tabulated_water
[]

[BCs]
  [./constant_injection_porepressure]
    type = PresetBC
    variable = porepressure
    value = 2E6
    boundary = injection_area
  [../]
  [./constant_injection_temperature]
    type = PresetBC
    variable = temperature
    value = 333
    boundary = injection_area
  [../]
[]

[Modules]
  [./FluidProperties]
    [./true_water]
      type = Water97FluidProperties
    [../]
    [./tabulated_water]
      type = TabulatedFluidProperties
      fp = true_water
      temperature_min = 275
      interpolated_properties = 'density viscosity enthalpy internal_energy'
      fluid_property_file = water97_tabulated.csv
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
    block = aquifer
    permeability = '1E-14 0 0   0 1E-14 0   0 0 1E-14'
  [../]
  [./permeability_caps]
    type = PorousFlowPermeabilityConst
    block = caps
    permeability = '1E-15 0 0   0 1E-15 0   0 0 1E-16'
  [../]

  [./thermal_expansion]
    type = PorousFlowConstantThermalExpansionCoefficient
    biot_coefficient = 0.8
    drained_coefficient = 0.003
    fluid_coefficient = 0.0002
  [../]
  [./porosity_nodal]
    type = PorousFlowPorosity
    porosity_zero = 0.1
    at_nodes = true
  [../]
  [./rock_internal_energy]
    type = PorousFlowMatrixInternalEnergy
    density = 2500.0
    specific_heat_capacity = 1200.0
  [../]
  [./thermal_conductivity]
    type = PorousFlowThermalConductivityIdeal
    dry_thermal_conductivity = '10 0 0  0 10 0  0 0 10'
    block = 'caps aquifer'
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
  end_time = 1E6
  dt = 1E5
  nl_abs_tol = 1E-10
[]

[Outputs]
  exodus = true
[]
