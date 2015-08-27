[Mesh]
  type = FileMesh
  file = line_source_cube.e
  dim = 2
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
    block = bulk
  [../]
  [./heating]
    type = UserForcingFunction
    variable = u
    function = 1
    block = heater
  [../]
[]

[BCs]
  [./outside]
    type = DirichletBC
    variable = u
    boundary = outside
    value = 0
  [../]
[]

[Materials]
  [./diffusivity]
    type = GenericConstantMaterial
    block = 'bulk heater'
    prop_names = diffusivity
    prop_values = 1
  [../]
[]

[Postprocessors]
  [./total_flux]
    type = SideFluxIntegral
    variable = u
    boundary = outside
    diffusivity = diffusivity
  [../]
[]

[Executioner]
  # Preconditioned JFNK (default)
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

