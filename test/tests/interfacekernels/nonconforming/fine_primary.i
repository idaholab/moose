# Same mesh and physics as coarse_primary.i, but primary_block (the side that carries the
# 'interface' boundary tag) is the finer 'gas' side instead. This direction already worked before
# the coarse-primary fix; it is kept as a regression guard and should give identical inventories
# to coarse_primary.i, since swapping which side is "primary" is purely a bookkeeping choice.
[Mesh]
  [base]
    type = CartesianMeshGenerator
    dim = 3
    dx = '1 1'
    ix = '1 1'
    dy = 1
    iy = 2
    dz = 1
    iz = 2
    subdomain_id = '0 1'
  []
  [refine]
    input = base
    type = RefineBlockGenerator
    block = 1
    refinement = 1
  []
  [interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = refine
    new_boundary = interface
    primary_block = 1
    paired_block = 0
  []
  [rename]
    input = interface
    type = RenameBlockGenerator
    old_block = '0 1'
    new_block = 'sol gas'
  []
[]

[Variables]
  [c_H2]
    family = LAGRANGE
    block = gas
  []
  [c_2H]
    family = LAGRANGE
    block = sol
  []
[]

[AuxVariables]
  [w]
    block = sol
  []
[]

[AuxKernels]
  [w]
    type = FunctionAux
    variable = w
    function = '1 + y + 2 * z'
    execute_on = INITIAL
  []
[]

[Kernels]
  [diff_gas]
    type = ADMatDiffusion
    variable = c_H2
    diffusivity = diffusivity
    block = gas
  []
  [diff_solid]
    type = ADMatDiffusion
    variable = c_2H
    diffusivity = diffusivity
    block = sol
  []
[]

[Materials]
  [gas_diff]
    type = ADGenericConstantMaterial
    prop_names = 'diffusivity'
    prop_values = 1
    block = gas
  []
  [solid_diff]
    type = ADGenericConstantMaterial
    prop_names = 'diffusivity'
    prop_values = 0.25
    block = sol
  []
  # The solid-side interface rate depends on an auxiliary variable, which must be evaluated at the
  # same physical points as the nonlinear variables on each face
  [solid_rate]
    type = ADParsedMaterial
    property_name = k_sol
    coupled_variables = w
    expression = 'w'
    block = sol
  []
  [gas_rate]
    type = ADGenericConstantMaterial
    prop_names = 'k_gas'
    prop_values = 2
    block = gas
  []
[]

[InterfaceKernels]
  [reaction]
    type = ADMatInterfaceReaction
    variable = c_H2
    neighbor_var = c_2H
    forward_rate = k_gas
    backward_rate = k_sol
    boundary = interface
  []
[]

[BCs]
  [left]
    type = ADDirichletBC
    variable = c_2H
    boundary = left
    value = 0
  []
  [right]
    type = ADDirichletBC
    variable = c_H2
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_abs_tol = 1e-12
[]

[Postprocessors]
  [steel_inventory]
    type = ADElementIntegralFunctorPostprocessor
    functor = c_2H
    block = sol
  []
  [gas_inventory]
    type = ADElementIntegralFunctorPostprocessor
    functor = c_H2
    block = gas
  []
  [total_inventory]
    type = LinearCombinationPostprocessor
    pp_coefs = '1 1'
    pp_names = 'steel_inventory gas_inventory'
  []
[]

[Outputs]
  csv = true
[]
