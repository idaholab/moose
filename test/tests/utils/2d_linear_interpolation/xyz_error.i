[Mesh]
  file = cube.e
  # This problem only has 1 element, so using DistributedMesh in parallel
  # isn't really an option, and we don't care that much about DistributedMesh
  # in serial.
  parallel_type = replicated
[]

[Variables]

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]


  [./u]
    type = PiecewiseBilinear
    #x = '0 1 3' # Testing this error
    y = '0 1 3'
    z = '0 0 0 0 1 3 0 5 7'
    axis = 0
  [../]
[] # End Functions

[Kernels]

  [./diffu]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]

  [./u]
    type = FunctionDirichletBC
    variable = u
    boundary = '1'
    function = u
  [../]
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 2
  nl_rel_tol = 1e-12
[]
