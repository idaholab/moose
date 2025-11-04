[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
[]

[Variables]
  [./u]
    type = MooseVariableFVReal
  [../]
[]

[FVKernels]
  [./diff]
    type = FVDiffusion
    variable = u
    coeff = 1
  [../]
  [force]
    type = FVCoupledForce
    v = v
    variable = u
  []
[]

[FunctorMaterials]
  [parsed]
    type = ADParsedFunctorMaterial
    property_name = 'v'
    functor_names = 'u'
    expression = 'if(u>0.1,1e6,0)'
  []
[]

[FVBCs]
  [./left]
    type = FVDirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = FVDirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  line_search = 'none'
  solve_type = NEWTON
  nl_max_its = 5
  nl_div_tol = 10
  petsc_options = '-snes_converged_reason -ksp_converged_reason'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]
