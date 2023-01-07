[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmin = -0.5
  xmax = 0.5
  ymin = -0.5
  ymax = 0.5
[]

[Variables]
  [scalar]
    order = THIRD
    family = SCALAR
  []
  [u]
    [InitialCondition]
      type = FunctionIC
      function = 'x*x+y*y'
    []
  []
[]

[Kernels]
  [u_dot]
    type = TimeDerivative
    variable = u
  []
  [c_res]
    type = Diffusion
    variable = u
  []
[]

[ScalarKernels]
  [d1]
    type = ODETimeDerivative
    variable = scalar
  []
[]

[BCs]
  [Periodic]
    [all]
      auto_direction = 'x y'
      variable = 'u'
    []
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -sub_pc_type'
  petsc_options_value = 'asm         lu     '
  num_steps = 2
[]

[Adaptivity]
  initial_steps = 2
  max_h_level = 2
  marker = EFM
  [Markers]
    [EFM]
      type = ErrorFractionMarker
      coarsen = 0.2
      refine = 0.8
      indicator = GJI
    []
  []
  [Indicators]
    [GJI]
      type = GradientJumpIndicator
      variable = u
    []
  []
[]

[Outputs]
  exodus = true
[]
