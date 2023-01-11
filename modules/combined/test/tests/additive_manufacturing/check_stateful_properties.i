[Problem]
  kernel_coverage_check = false
  material_coverage_check = false
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = 0
    xmax = 10
    ymin = 0
    ymax = 10
    zmin = 0
    zmax = 0.5
    nx = 20
    ny = 20
    nz = 1
  []
  [left_domain]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0 0 0'
    top_right = '5 10 0.5'
    block_id = 1
  []
  [right_domain]
    input = left_domain
    type = SubdomainBoundingBoxGenerator
    bottom_left = '5 0 0'
    top_right = '10 10 0.5'
    block_id = 2
  []
  [sidesets]
    input = right_domain
    type = SideSetsAroundSubdomainGenerator
    normal = '1 0 0'
    block = 1
    new_boundary = 'moving_interface'
  []
[]

[Variables]
  [temp]
    initial_condition = 300
    block = '1'
  []
[]

# Output aux variables to check if stateful properties
# are initialized properly for newly added elements
[AuxVariables]
  [density_aux]
    order = CONSTANT
    family = MONOMIAL
    block = '1'
  []
  [specific_heat_aux]
    order = CONSTANT
    family = MONOMIAL
    block = '1'
  []
  [thermal_conductivity_aux]
    order = CONSTANT
    family = MONOMIAL
    block = '1'
  []
[]

[Kernels]
  [null]
    type = NullKernel
    variable = temp
    jacobian_fill = 1e-5
  []
[]

[AuxKernels]
  [density]
    type = ADMaterialRealAux
    property = density
    variable = density_aux
    block = 1
  []
  [specific_heat]
    type = ADMaterialRealAux
    property = specific_heat
    variable = specific_heat_aux
    block = 1
  []
  [thermal_conductivity]
    type = ADMaterialRealAux
    property = thermal_conductivity
    variable = thermal_conductivity_aux
    block = 1
  []
[]

[Functions]
  [fx]
    type = ParsedFunction
    expression = '5.25'
  []
  [fy]
    type = ParsedFunction
    expression = '2.5*t'
  []
  [fz]
    type = ParsedFunction
    expression = '0.25'
  []
[]

[Materials]
  [density]
    type = ADDensity
    density = 4.43e-6
    block = '1'
  []
  [heat]
    type = ADHeatConductionMaterial
    specific_heat = 600
    thermal_conductivity = 10e-3
    block = '1'
  []
  [volumetric_heat]
    type = ADGenericConstantMaterial
    prop_names = 'volumetric_heat'
    prop_values = 100
    block = '1'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient

  automatic_scaling = true

  solve_type = 'NEWTON'

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  line_search = 'none'

  l_max_its = 10
  nl_max_its = 20
  nl_rel_tol = 1e-4

  start_time = 0.0
  end_time = 1.0
  dt = 1e-1
  dtmin = 1e-4
[]

[UserObjects]
  [activated_elem_uo]
    type = ActivateElementsByPath
    execute_on = timestep_begin
    function_x = fx
    function_y = fy
    function_z = fz
    active_subdomain_id = 1
    expand_boundary_name = 'moving_interface'
  []
[]

[Outputs]
  exodus = true
[]
