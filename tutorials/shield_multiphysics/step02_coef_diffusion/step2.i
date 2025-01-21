# Start by running meshing script
!include ../step01_diffusion/mesh.i

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
  []
  [diffusion_concrete]
    type = CoefDiffusion
    variable = T
    coef = 2.25
  []
[]

[BCs]
  [left]
    type = DirichletBC  # Simple u=value BC
    variable = T        # Variable to be set
    boundary = left     # Name of a sideset in the mesh
    value = 330
  []
  [right]
    type = DirichletBC
    variable = T
    boundary = right
    value = 300
  []
[]

[Problem]
  type = FEProblem  # This is the "normal" type of Finite Element Problem in MOOSE
[]

[Executioner]
  type = Steady       # Steady state problem
  solve_type = NEWTON # Perform a Newton solve, uses AD to compute Jacobian terms
  petsc_options_iname = '-pc_type -pc_hypre_type' # PETSc option pairs with values below
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true # Output Exodus format
[]
