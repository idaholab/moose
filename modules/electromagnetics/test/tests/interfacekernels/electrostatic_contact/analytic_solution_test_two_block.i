# Regression test for ElectrostaticContactCondition with analytic solution with
# two blocks
#
# dim = 1D
# X = [0,2]
# Interface at X = 1
#
#   stainless_steel        graphite
# +------------------+------------------+
#
# Left BC: Potential = 1
# Right BC: Potential = 0
# Center Interface: ElectrostaticContactCondition
#

[Mesh]
  [line]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 4
    xmax = 2
  []
  [break]
    type = SubdomainBoundingBoxGenerator
    input = line
    block_id = 1
    block_name = 'graphite'
    bottom_left = '1 0 0'
    top_right = '2 0 0'
  []
  [block_rename]
    type = RenameBlockGenerator
    input = break
    old_block = 0
    new_block = 'stainless_steel'
  []
  [interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = block_rename
    primary_block = 'stainless_steel'
    paired_block = 'graphite'
    new_boundary = 'ssg_interface'
  []
[]

[Variables]
  [potential_graphite]
    block = graphite
  []
  [potential_stainless_steel]
    block = stainless_steel
  []
[]

[AuxVariables]
  [analytic_potential_stainless_steel]
    block = stainless_steel
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
  [electric_stainless_steel]
    type = ADMatDiffusion
    variable = potential_stainless_steel
    diffusivity = electrical_conductivity
    block = stainless_steel
  []
[]

[AuxKernels]
  [analytic_function_aux_stainless_steel]
    type = FunctionAux
    function = potential_fxn_stainless_steel
    variable = analytic_potential_stainless_steel
    block = stainless_steel
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
    variable = potential_stainless_steel
    boundary = left
    value = 1
  []
  [elec_right]
    type = ADDirichletBC
    variable = potential_graphite
    boundary = right
    value = 0
  []
[]

[InterfaceKernels]
  [electric_contact_conductance_ssg]
    type = ElectrostaticContactCondition
    variable = potential_stainless_steel
    neighbor_var = potential_graphite
    boundary = ssg_interface
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
  [sigma_stainless_steel]
    type = ADGenericConstantMaterial
    prop_names = electrical_conductivity
    prop_values = 1.41867e6
    block = stainless_steel
  []

  # harmonic mean of graphite and stainless steel hardness
  [mean_hardness]
    type = ADGenericConstantMaterial
    prop_names = mean_hardness
    prop_values = 2.4797e9
  []
[]

[Functions]
  [potential_fxn_stainless_steel]
    type = ElectricalContactTestFunc
    domain = stainless_steel
  []
  [potential_fxn_graphite]
    type = ElectricalContactTestFunc
    domain = graphite
  []
[]

[Postprocessors]
  [error_stainless_steel]
    type = ElementL2Error
    variable = potential_stainless_steel
    function = potential_fxn_stainless_steel
    block = stainless_steel
  []
  [error_graphite]
    type = ElementL2Error
    variable = potential_graphite
    function = potential_fxn_graphite
    block = graphite
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
