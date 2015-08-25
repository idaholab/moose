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
  active = 'u'

  [./u]
    # second order is way better than first order
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff rea rhs'
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./rea]
    type = CoefReaction
    variable = u
    coefficient = 2.0
  [../]

  [./rhs]
    type = MassEigenKernel
    variable = u
    eigen = false
  [../]

  [./rea1]
    type = CoefReaction
    variable = u
    coefficient = 1.0
  [../]
[]

[BCs]
  [./inhomogeneous]
    type = DirichletBC
    variable = u
    boundary = '2 3'
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Postprocessors]
  active = 'unorm'

  [./unorm]
    type = ElementIntegralVariablePostprocessor
    variable = u
    execute_on = timestep_end
  [../]
[]

[Outputs]
  file_base = normal_eigen_kernel
  exodus = true
[]
