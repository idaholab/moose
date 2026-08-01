# A rotated mesh makes the boundary-normal flux depend on both principal diffusion coefficients,
# exercising the anisotropic Neumann path rather than an axis-aligned special case.
# Thirty degrees avoids axis alignment while keeping the transformed normal easy to verify.
rotation_degrees = 30
# Two cells per direction are sufficient to exercise both boundary and internal face assembly.
cells_per_direction = 2
# Unequal coefficients and nonzero gradient components expose omitted or duplicated tensor terms.
diffusion_x = 2
diffusion_y = 3
gradient_x = 1
gradient_y = 2
rotation_radians = ${fparse rotation_degrees * pi / 180}
flux_x = ${fparse diffusion_x * gradient_x}
flux_y = ${fparse diffusion_y * gradient_y}
right_flux = ${fparse flux_x * cos(rotation_radians) + flux_y * sin(rotation_radians)}
top_flux = ${fparse -flux_x * sin(rotation_radians) + flux_y * cos(rotation_radians)}
bottom_flux = ${fparse -top_flux}

[Mesh]
  [generated]
    type = GeneratedMeshGenerator
    dim = 2
    nx = ${cells_per_direction}
    ny = ${cells_per_direction}
  []
  [rotate]
    type = TransformGenerator
    input = generated
    transform = ROTATE
    vector_value = '0 0 ${rotation_degrees}'
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
  [left]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = left
    functor = exact_solution
  []
  [right]
    type = LinearFVAdvectionDiffusionFunctorNeumannBC
    variable = u
    boundary = right
    functor = ${right_flux}
  []
  [top]
    type = LinearFVAdvectionDiffusionFunctorNeumannBC
    variable = u
    boundary = top
    functor = ${top_flux}
  []
  [bottom]
    type = LinearFVAdvectionDiffusionFunctorNeumannBC
    variable = u
    boundary = bottom
    functor = ${bottom_flux}
  []
[]

[FunctorMaterials]
  [diffusion_tensor]
    type = GenericVectorFunctorMaterial
    prop_names = diffusivity_tensor
    prop_values = '${diffusion_x} ${diffusion_y} 0'
  []
[]

[Functions]
  [exact_solution]
    type = ParsedFunction
    expression = '${gradient_x} * x + ${gradient_y} * y'
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
  # This isolates the boundary-discretization error from the iterative linear-solver tolerance.
  l_abs_tol = 1e-12
[]

[Outputs]
  csv = true
  execute_on = FINAL
[]
