# Darcy flow
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
  [../]
[]

[PorousFlowBasicTHM]
  porepressure = porepressure
  coupling_type = Hydro
  gravity = '0 0 0'
  fp = the_simple_fluid
[]

[BCs]
  [./constant_injection_porepressure]
    type = PresetBC
    variable = porepressure
    value = 1E6
    boundary = injection_area
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
    block = aquifer
    permeability = '1E-14 0 0   0 1E-14 0   0 0 1E-14'
  [../]
  [./permeability_caps]
    type = PorousFlowPermeabilityConst
    block = caps
    permeability = '1E-15 0 0   0 1E-15 0   0 0 1E-16'
  [../]
[]

[Preconditioning]
  active = basic
  [./basic]
    type = SMP
    full = true
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
  nl_abs_tol = 1E-13
[]

[Outputs]
  exodus = true
[]

