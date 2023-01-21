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
  [./mat_props]
    type = GenericConstantMaterial
    block = 0
    prop_names = diffusivity
    prop_values = 2
  [../]

  [./mat_props_bnd]
    type = GenericConstantMaterial
    boundary = right
    prop_names = diffusivity
    prop_values = 1
  [../]

  [./mat_props_vector]
    type = GenericConstantVectorMaterial
    boundary = 'right top'
    prop_names = diffusivity_vec
    prop_values = '1 1.5 1'
  [../]
[]

[Postprocessors]
  inactive = 'avg_flux_top'
  [./avg_flux_right]
    # Computes -\int(exp(y)+1) from 0 to 1 which is -2.718281828
    type = SideDiffusiveFluxIntegral
    variable = u
    boundary = right
    diffusivity = diffusivity
  [../]
  [./avg_flux_top]
    type = SideVectorDiffusivityFluxIntegral
    variable = u
    boundary = top
    diffusivity = diffusivity_vec
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
