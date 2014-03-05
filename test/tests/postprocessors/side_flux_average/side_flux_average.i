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
    type = ParsedFunction
    value = exp(y)+1
  [../]
  [./source_func]
    type = ParsedFunction
    value = -x*exp(y)*pow(ln(exp()),2)
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./source]
    type = BodyForce
    variable = u
    function = source_func
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
    type = FunctionDirichletBC
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
    prop_values = 1
  [../]
[]

[Postprocessors]
  [./avg_flux_right]
    type = SideFluxAverage
    variable = u
    boundary = right
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
  output_initial = true
  exodus = true
  [./console]
    type = Console
    perf_log = true
    linear_residuals = true
  [../]
[]
