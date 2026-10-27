# A diffusion coefficient equal to x is zero only at the left boundary and remains positive on
# every interior face, allowing the constant solution to test one-term boundary reconstruction.
cells = 2

[Mesh]
  [generated]
    type = GeneratedMeshGenerator
    dim = 1
    nx = ${cells}
  []
[]

[Problem]
  linear_sys_names = u_system
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    solver_sys = u_system
  []
[]

[LinearFVKernels]
  [diffusion]
    type = LinearFVAnisotropicDiffusion
    variable = u
    diffusion_tensor = diffusivity_tensor
  []
[]

[LinearFVBCs]
  [zero_flux]
    type = LinearFVAnisotropicDiffusionFunctorNeumannBC
    variable = u
    boundary = left
    functor = 0
    diffusion_tensor = diffusivity_tensor
    use_two_term_expansion = false
  []
  [fixed_value]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = right
    functor = exact_solution
  []
[]

[FunctorMaterials]
  [diffusion_tensor]
    type = GenericVectorFunctorMaterial
    prop_names = diffusivity_tensor
    prop_values = 'diffusion_x 0 0'
  []
[]

[Functions]
  [diffusion_x]
    type = ParsedFunction
    expression = x
  []
  [exact_solution]
    type = ParsedFunction
    expression = 1
  []
[]

[Postprocessors]
  [error]
    type = ElementL2FunctorError
    approximate = u
    exact = exact_solution
  []
[]

[Executioner]
  type = Steady
  system_names = u_system
  # A direct solve isolates the boundary reconstruction from iterative solver tolerances.
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  csv = true
  execute_on = FINAL
[]
