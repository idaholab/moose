# The variable lives on block 1 only and is bounded by an internal sideset shared with block 2.
# The boundary functors are functor materials defined block by block, so the boundary condition
# must evaluate them on the variable side of the internal sideset.
#
# Dirichlet case: the boundary value is 1 on block 1 and 5 on block 2. With u(0) = 0, the exact
# solution on block 1 is u = x, which a two-point flux reproduces exactly; interpolating the two
# sides' values would raise the boundary value above 1.
#
# Anisotropic Neumann case: the flux is defined on block 2 only, which the boundary condition
# must find on the other side of the internal sideset. With the unit diffusivity, the exact
# solution is u = flux * x.

[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 1
    dx = '1 1'
    ix = '4 4'
    subdomain_id = '1 2'
  []
  [interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = cmg
    new_boundary = interface
    primary_block = 1
    paired_block = 2
  []
[]

[Problem]
  linear_sys_names = 'u_sys'
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    solver_sys = u_sys
    initial_condition = 0
    block = 1
  []
[]

[LinearFVKernels]
  active = 'diffusion'
  [diffusion]
    type = LinearFVDiffusion
    variable = u
  []
  [anisotropic_diffusion]
    type = LinearFVAnisotropicDiffusion
    variable = u
    diffusion_tensor = unit_tensor
  []
[]

[LinearFVBCs]
  active = 'left dirichlet'
  [left]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = left
    functor = 0
  []
  [dirichlet]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = interface
    functor = boundary_value
  []
  [anisotropic_neumann]
    type = LinearFVAnisotropicDiffusionFunctorNeumannBC
    variable = u
    boundary = interface
    functor = boundary_flux
    diffusion_tensor = unit_tensor
  []
[]

[FunctorMaterials]
  [boundary_value]
    type = PiecewiseByBlockFunctorMaterial
    prop_name = boundary_value
    subdomain_to_prop_value = '1 1 2 5'
  []
  [unit_tensor]
    type = GenericVectorFunctorMaterial
    prop_names = unit_tensor
    prop_values = '1 1 1'
  []
  [boundary_flux]
    type = GenericFunctorMaterial
    prop_names = boundary_flux
    prop_values = 2
    block = 2
  []
[]

[Functions]
  [exact]
    type = ParsedFunction
    expression = 'x'
  []
[]

[Postprocessors]
  [error]
    type = ElementL2FunctorError
    approximate = u
    exact = exact
    block = 1
  []
[]

[Executioner]
  type = Steady
  system_names = u_sys
  l_tol = 1e-12
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  csv = true
[]
