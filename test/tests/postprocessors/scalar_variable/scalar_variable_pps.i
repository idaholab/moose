[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [time]
    type = TimeDerivative
    variable = u
  []
[]

[ScalarKernels]
  [time]
    type = ODETimeDerivative
    variable = v
  []
  [flux_sink]
    type = PostprocessorSinkScalarKernel
    variable = v
    postprocessor = scale_flux
  []
[]

[BCs]
  [right]
    type = DirichletBC
    value = 0
    variable = u
    boundary = 'right'
  []
  [left]
    type = ADMatchedScalarValueBC
    variable = u
    v = v
    boundary = 'left'
  []
[]

[Variables]
  [u][]
  [v]
    family = SCALAR
    order = FIRST
    initial_condition = 1
  []
[]

[Postprocessors]
  [flux]
    type = SideDiffusiveFluxIntegral
    variable = u
    diffusivity = 1
    boundary = 'left'
    execute_on = 'initial nonlinear linear timestep_end'
  []
  [scale_flux]
    type = ScalePostprocessor
    scaling_factor = -1
    value = flux
    execute_on = 'initial nonlinear linear timestep_end'
  []
  [reporter]
    type = ScalarVariable
    variable = v
    execute_on = 'initial timestep_end'
  []
[]

[Executioner]
  type = Transient
  dt = .1
  end_time = 1
  solve_type = PJFNK
  nl_rel_tol = 1e-12
[]

[Outputs]
  csv = true
[]
