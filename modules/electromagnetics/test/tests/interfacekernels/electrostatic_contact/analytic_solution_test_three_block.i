# Regression test for ElectrostaticContactCondition with analytic solution with
# three blocks
#
# dim = 1D
# X = [0,3]
# Interfaces at X = 1 and X = 2
#
#   stainless_steel        graphite        stainless_steel
# +------------------+------------------+------------------+
#
# Left BC: Potential = 1
# Right BC: Potential = 0
# Left Interface: ElectrostaticContactCondition (primary = stainless_steel)
# Right Interface: ElectrostaticContactCondition (primary = graphite)
#

[Mesh]
  [line]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 6
    xmax = 3
  []
  [break_center]
    type = SubdomainBoundingBoxGenerator
    input = line
    block_id = 1
    block_name = 'graphite'
    bottom_left = '1 0 0'
    top_right = '2 0 0'
  []
  [break_right]
    type = SubdomainBoundingBoxGenerator
    input = break_center
    block_id = 2
    bottom_left = '2 0 0'
    top_right = '3 0 0'
  []
  [ssg_interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = break_right
    primary_block = 0
    paired_block = 1
    new_boundary = 'ssg_interface'
  []
  [gss_interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = ssg_interface
    primary_block = 1
    paired_block = 2
    new_boundary = 'gss_interface'
  []
  [block_rename]
    type = RenameBlockGenerator
    input = gss_interface
    old_block = '0 2'
    new_block = 'stainless_steel_left stainless_steel_right'
  []
[]

[Variables]
  [potential_graphite]
    block = graphite
  []
  [potential_stainless_steel_left]
    block = stainless_steel_left
  []
  [potential_stainless_steel_right]
    block = stainless_steel_right
  []
[]

[AuxVariables]
  [analytic_potential_stainless_steel_left]
    block = stainless_steel_left
  []
  [analytic_potential_stainless_steel_right]
    block = stainless_steel_right
  []
  [analytic_potential_graphite]
    block = graphite
  []
[]

[Kernels]
  [electric_graphite]
    type = ADMatDiffusion
    variable = potential_graphite
    diffusivity = electrical_conductivity
    block = graphite
  []
  [electric_stainless_steel_left]
    type = ADMatDiffusion
    variable = potential_stainless_steel_left
    diffusivity = electrical_conductivity
    block = stainless_steel_left
  []
  [electric_stainless_steel_right]
    type = ADMatDiffusion
    variable = potential_stainless_steel_right
    diffusivity = electrical_conductivity
    block = stainless_steel_right
  []
[]

[AuxKernels]
  [analytic_function_aux_stainless_steel_left]
    type = FunctionAux
    function = potential_fxn_stainless_steel_left
    variable = analytic_potential_stainless_steel_left
    block = stainless_steel_left
  []
  [analytic_function_aux_stainless_steel_right]
    type = FunctionAux
    function = potential_fxn_stainless_steel_right
    variable = analytic_potential_stainless_steel_right
    block = stainless_steel_right
  []
  [analytic_function_aux_graphite]
    type = FunctionAux
    function = potential_fxn_graphite
    variable = analytic_potential_graphite
    block = graphite
  []
[]

[BCs]
  [elec_left]
    type = ADDirichletBC
    variable = potential_stainless_steel_left
    boundary = left
    value = 1
  []
  [elec_right]
    type = ADDirichletBC
    variable = potential_stainless_steel_right
    boundary = right
    value = 0
  []
[]

[InterfaceKernels]
  [electric_contact_conductance_ssg]
    type = ElectrostaticContactCondition
    variable = potential_stainless_steel_left
    neighbor_var = potential_graphite
    boundary = ssg_interface
    mean_hardness = mean_hardness
    mechanical_pressure = 3000
  []
  [electric_contact_conductance_gss]
    type = ElectrostaticContactCondition
    variable = potential_graphite
    neighbor_var = potential_stainless_steel_right
    boundary = gss_interface
    mean_hardness = mean_hardness
    mechanical_pressure = 3000
  []
[]

[Materials]
  #graphite (at 300 K)
  [sigma_graphite]
    type = ADGenericConstantMaterial
    prop_names = electrical_conductivity
    prop_values = 73069.2
    block = graphite
  []

  #stainless_steel (at 300 K)
  [sigma_stainless_steel_left]
    type = ADGenericConstantMaterial
    prop_names = electrical_conductivity
    prop_values = 1.41867e6
    block = stainless_steel_left
  []
  [sigma_stainless_steel_right]
    type = ADGenericConstantMaterial
    prop_names = electrical_conductivity
    prop_values = 1.41867e6
    block = stainless_steel_right
  []

  # harmonic mean of graphite and stainless steel hardness
  [mean_hardness]
    type = ADGenericConstantMaterial
    prop_names = mean_hardness
    prop_values = 2.4797e9
  []
[]

[Functions]
  [potential_fxn_stainless_steel_left]
    type = ElectricalContactTestFunc
    domain = stainless_steel
    three_block = true
    three_block_side = left
  []
  [potential_fxn_stainless_steel_right]
    type = ElectricalContactTestFunc
    domain = stainless_steel
    three_block = true
    three_block_side = right
  []
  [potential_fxn_graphite]
    type = ElectricalContactTestFunc
    domain = graphite
    three_block = true
  []
[]

[Postprocessors]
  [error_stainless_steel_left]
    type = ElementL2Error
    variable = potential_stainless_steel_left
    function = potential_fxn_stainless_steel_left
    block = stainless_steel_left
  []
  [error_graphite]
    type = ElementL2Error
    variable = potential_graphite
    function = potential_fxn_graphite
    block = graphite
  []
  [error_stainless_steel_right]
    type = ElementL2Error
    variable = potential_stainless_steel_right
    function = potential_fxn_stainless_steel_right
    block = stainless_steel_right
  []
[]


[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  automatic_scaling = true
[]

[Outputs]
  csv = true
  perf_graph = true
[]
