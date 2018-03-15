[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  nz = 0
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff'

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 1
  [../]
[]

[Materials]
  [./linear_interp]
    type = LinearInterpolationMaterial
    prop_name = 'diffusivity'
    independent_vals = '0 0.2 0.2 0.4 0.6 0.8 1.0'
    dependent_vals = '16 8 4 2 1 0.5 1'

    # Note the following line gets enabled by the tester
    #use_poly_fit = true

    block = 0
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = out
  gmv = true
[]
