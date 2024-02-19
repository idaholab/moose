[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 160
    ymin = 0
    ymax = 160
    nx = 8
    ny = 8
  []
  uniform_refine = 0
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diffusion]
    type = MatDiffusion
    diffusivity = diffusivity
    variable = u
  []
  [reaction]
    type = CoefReaction
    coefficient = 0.01
    variable = u
  []
  [rhs]
    type = CoefReaction
    extra_vector_tags = 'eigen'
    coefficient = -0.01
    variable = u
  []
[]

[BCs]
  [robin]
    type = VacuumBC
    boundary = 'left bottom'
    variable = u
  []
[]

[Materials]
  [nm]
    type = GenericConstantMaterial
    block = 0
    prop_names = 'diffusivity'
    prop_values = 0.333333333333333333
  []
[]

[VectorPostprocessors]
  [eigen]
    type = Eigenvalues
    inverse_eigenvalue = true
  []
[]

[Postprocessors]
  [fluxintegral]
    type = ElementIntegralVariablePostprocessor
    variable = u
    execute_on = linear
  []
[]

[Problem]
  type = EigenProblem
[]

[Executioner]
  type = Eigenvalue
  solve_type = PJFNK
  free_power_iterations = 4
  nl_abs_tol = 2e-10
[]

[Outputs]
  csv = true
[]
