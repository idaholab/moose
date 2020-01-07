[Mesh]
 type = GeneratedMesh
 dim = 2
 xmin = 0
 xmax = 10
 ymin = 0
 ymax = 10
 elem_type = QUAD4
 nx = 8
 ny = 8

 uniform_refine = 0
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
  [./v]
    order = FIRST
    family = LAGRANGE
    eigen = true
  [../]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]
  [./diff_v]
    type = Diffusion
    variable = v
  [../]

  [./rhs]
    type = CoupledEigenKernel
    variable = u
    v = v
  [../]
  [./src_v]
    type = CoupledForce
    variable = v
    v = u
  [../]
[]

[BCs]
  [./homogeneous_u]
    type = DirichletBC
    variable = u
    boundary = '0 1 2 3'
    value = 0
  [../]
  [./homogeneous_v]
    type = DirichletBC
    variable = v
    boundary = '0 1 2 3'
    value = 0
  [../]
[]

[Executioner]
  type = NonlinearEigen

  bx_norm = 'vnorm'

  free_power_iterations = 2
  nl_abs_tol = 1e-12
  nl_rel_tol = 1e-50
  k0 = 1.0
  output_after_power_iterations = false

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Postprocessors]
  [./vnorm]
    type = ElementIntegralVariablePostprocessor
    variable = v
    # execute on residual is important for nonlinear eigen solver!
    execute_on = linear
  [../]

  [./udiff]
    type = ElementL2Diff
    variable = u
    outputs = console
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = ne_deficient_b
  exodus = true
[]
