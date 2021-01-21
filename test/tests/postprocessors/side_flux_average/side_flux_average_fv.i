[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  [../]
[]

[Functions]
  [./right_bc]
    # Flux BC for computing the analytical solution in the postprocessor
    type = ParsedFunction
    value = exp(y)+1
  [../]
[]

[FVKernels]
  [./diff]
    type = FVDiffusion
    variable = u
    coeff = 1
  [../]
[]

[FVBCs]
  [./left]
    type = FVDirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = FVDiffusionFunctionBC
    variable = u
    boundary = right
    function = right_bc
  [../]
[]

[Materials]
  [./mat_props]
    type = ADGenericConstantMaterial
    block = 0
    prop_names = diffusivity
    prop_values = 2
  [../]

  [./mat_props_bnd]
    type = ADGenericConstantMaterial
    boundary = right
    prop_names = diffusivity
    prop_values = 1
  [../]
[]

[Postprocessors]
  [./avg_flux_right]
    # Computes -\int(exp(y)+1) from 0 to 1 which is -2.718281828
    type = SideFluxAverage
    variable = u
    boundary = right
    diffusivity = diffusivity
    fv = true
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
