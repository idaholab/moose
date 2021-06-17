[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
  xmax = 5
  ymax = 5
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
  [v]
    order = FIRST
    family = LAGRANGE
  []
[]

[BCs]
  [u_left]
    type = DirichletBC
    variable = u
    value = 0
    boundary = 'left'
  []
  [u_right]
    type = DirichletBC
    variable = u
    value = 1
    boundary = 'right'
  []
  [v_left]
    type = DirichletBC
    variable = v
    value = 0
    boundary = 'left'
  []
  [v_right]
    type = DirichletBC
    variable = v
    value = 1
    boundary = 'right'
  []
[]

[Kernels]
  [diffusion_u]
    type = Diffusion
    variable = u
  []
  [diffusion_v]
    type = Diffusion
    variable = v
  []
[]

[RayKernels]
  active = 'source'

  [source]
    type = CoupledLineSourceRayKernelTest
    variable = u
    coupled = v
  []
  [source_ad]
    type = ADCoupledLineSourceRayKernelTest
    variable = u
    coupled = v
  []
[]

[UserObjects/study]
  type = RepeatableRayStudy
  start_points = '0 0 0
                  0 4.9 0'
  end_points = '5 0 0
                4.9 3.9 0'
  names = 'ray1 ray2'
  execute_on = PRE_KERNELS
[]

[Preconditioning/smp]
  type = SMP
  full = true
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
