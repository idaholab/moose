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
    type = MassEigenKernel
    coefficient = 0.01
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

[Postprocessors]
  [fluxintegral]
    type = ElementIntegralVariablePostprocessor
    variable = u
    execute_on = linear
  []
[]

[Executioner]
  type = NonlinearEigen
  bx_norm = fluxintegral
  solve_type = PJFNK
  free_power_iterations = 4
  nl_abs_tol = 2e-10
  output_after_power_iterations = false
[]

[Outputs]
  csv = true
[]
