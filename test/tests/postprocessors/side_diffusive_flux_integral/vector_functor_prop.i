[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
[]

[Functions]
  [./right_bc]
    # Flux BC for computing the analytical solution in the postprocessor
    type = ParsedFunction
    expression = exp(y)+1
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = FunctionNeumannBC
    variable = u
    boundary = right
    function = right_bc
  [../]
[]

[Materials]
  [./mat_props_vector_functor]
    type = ADGenericVectorFunctorMaterial
    prop_names = diffusivity_vec
    prop_values = '1 1.5 1'
  [../]
  [conversion]
    type = PropFromFunctorProp
    vector_functor = diffusivity_vec
    vector_prop = diffusivity_vec
  []
[]

[Postprocessors]
  [./avg_flux_right]
    # Computes -\int(exp(y)+1) from 0 to 1 which is -2.718281828
    type = ADSideVectorDiffusivityFluxIntegral
    variable = u
    boundary = right
    diffusivity = diffusivity_vec
  [../]
  [./avg_flux_top]
    type = ADSideVectorDiffusivityFluxIntegral
    variable = u
    boundary = top
    diffusivity = diffusivity_vec
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
