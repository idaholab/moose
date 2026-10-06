# The 'gas' block is statically refined one level beyond 'sol', so each 'sol' element's face on
# the interface borders four active 'gas' children instead of one. primary_block (the side that
# carries the 'interface' boundary tag, and thus the side InterfaceKernels are evaluated on) is
# the coarser 'sol' side here, exercising the one-coarse-primary-to-many-fine-neighbors case.
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
    primary_block = 0
    paired_block = 1
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
    prop_values = 1
    block = sol
  []
[]

[InterfaceKernels]
  [penalty_diff]
    type = ADPenaltyInterfaceDiffusion
    variable = c_2H
    neighbor_var = c_H2
    penalty = 1
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
