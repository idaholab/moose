[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    xmax = 2
    ny = 2
    ymax = 2
  []
  [./subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0 0 0'
    top_right = '1 1 0'
    block_id = 1
  [../]
  [./primary0_interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = subdomain1
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'primary0_interface'
  [../]
  [./break_boundary]
    input = primary0_interface
    type = BreakBoundaryOnSubdomainGenerator
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    block = 0
  [../]

  [./v]
    order = FIRST
    family = LAGRANGE
    block = 1
  [../]
[]


[Kernels]
  [./diff_u]
    type = CoeffParamDiffusion
    variable = u
    D = 2
    block = 0
  [../]
  [./diff_v]
    type = CoeffParamDiffusion
    variable = v
    D = 4
    block = 1
  [../]
  [./source_u]
    type = BodyForce
    variable = u
    function = 0.1*t
  [../]
[]

[InterfaceKernels]
  [./primary0_interface]
    type = PenaltyInterfaceDiffusionDot
    variable = u
    neighbor_var = v
    boundary = primary0_interface
    penalty = 1e6
  [../]
[]

[BCs]
  [./u]
    type = VacuumBC
    variable = u
    boundary = 'left_to_0 bottom_to_0 right top'
  [../]
  [./v]
    type = VacuumBC
    variable = v
    boundary = 'left_to_1 bottom_to_1'
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = TRUE
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  dt = 0.1
  num_steps = 3
  dtmin = 0.1
  line_search = none
[]

[Outputs]
  exodus = true
[]


[UserObjects]
  [./interface_material_uo]
    type = InterfaceUserObjectTestGetMaterialProperty
    property = 'primary_prop'
    property_neighbor = 'secondary_prop'
    property_boundary = 'boundary_prop'
    property_interface = 'interface_prop'
    boundary = 'primary0_interface'
    execute_on = 'INITIAL LINEAR NONLINEAR TIMESTEP_BEGIN TIMESTEP_END FINAL'
  [../]
[]

[Materials]
  [./mat_primary]
    type = LinearNonLinearIterationMaterial
    block = 0
    prefactor = 1
    prop_name = 'primary_prop'
  [../]
  [./mat_secondary]
    type = LinearNonLinearIterationMaterial
    block = 1
    prefactor = 2
    prop_name = 'secondary_prop'
  [../]
  [./mat_boundary]
    type = LinearNonLinearIterationMaterial
    prefactor = 3
    boundary = 'primary0_interface'
    prop_name = 'boundary_prop'
  [../]
  [./mat_interface]
    type = LinearNonLinearIterationInterfaceMaterial
    prefactor = 4
    boundary = 'primary0_interface'
    prop_name = 'interface_prop'
  [../]
[]
