[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = '../step01_diffusion/mesh_in.e'
  []
[]

[Variables]
  [T]
    # Adds a Linear Lagrange variable by default
  []
[]

[Kernels]
  [diffusion_water]
    type = CoefDiffusion
    variable = T
    coef = 0.6
    block = water
  []
  [diffusion_concrete_hd]
    type = CoefDiffusion
    variable = T
    coef = 5
    block = concrete_hd
  []
  [diffusion_concrete]
    type = CoefDiffusion
    variable = T
    coef = 2.25
    block = concrete
  []
  [diffusion_Al]
    type = CoefDiffusion
    variable = T
    coef = 175
    block = Al
  []
[]

[BCs]
  [left]
    type = DirichletBC  # Simple u=value BC
    variable = T        # Variable to be set
    boundary = top      # Name of a sideset in the mesh
    value = 330
  []
  [right]
    type = DirichletBC
    variable = T
    boundary = bottom
    value = 300
  []
[]

[Executioner]
  type = Steady       # Steady state problem
  solve_type = NEWTON # Perform a Newton solve
  petsc_options_iname = '-pc_type -pc_hypre_type' # PETSc option pairs with values below
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true # Output Exodus format
[]
