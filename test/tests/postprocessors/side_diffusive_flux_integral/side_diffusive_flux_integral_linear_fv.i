[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Problem]
  linear_sys_names = 'u_sys'
[]

[Variables]
  [u]
    solver_sys = 'u_sys'
    type = MooseLinearVariableFVReal
  []
[]

[LinearFVKernels]
  [diff]
    type = LinearFVDiffusion
    variable = u
    diffusion_coeff = diffusivity
  []
[]

[LinearFVBCs]
  [left]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = left
    functor = 0
  []
  [right]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = right
    functor = 1
  []
[]

[FunctorMaterials]
  [mat_props]
    type = GenericFunctorMaterial
    prop_names = diffusivity
    prop_values = 1
  []
[]

[AuxVariables]
  [grad]
    family = MONOMIAL_VEC
    order = CONSTANT
  []
[]

[AuxKernels]
  [grad]
    variable = grad
    functor = u
    type = FunctorElementalGradientAux
  []
[]

[Postprocessors]
  [avg_left]
    type = SideAverageValue
    variable = u
    boundary = left
  []
  [avg_right]
    type = SideAverageValue
    variable = u
    boundary = right
  []
  [avg_flux_left]
    type = SideDiffusiveFluxAverage
    variable = u
    boundary = left
    functor_diffusivity = diffusivity
  []
  [avg_flux_right]
    type = SideDiffusiveFluxAverage
    variable = u
    boundary = right
    functor_diffusivity = diffusivity
  []
[]

[Executioner]
  type = Steady
  system_names = u_sys
  l_tol = 1e-10
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
