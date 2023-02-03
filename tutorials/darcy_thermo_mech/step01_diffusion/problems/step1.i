[Mesh]
  [gmg]
    type = GeneratedMeshGenerator # Can generate simple lines, rectangles and rectangular prisms
    dim = 2                       # Dimension of the mesh
    nx = 100                      # Number of elements in the x direction
    ny = 10                       # Number of elements in the y direction
    xmax = 0.304                  # Length of test chamber
    ymax = 0.0257                 # Test chamber radius
  []
  coord_type = RZ                 # Axisymmetric RZ
  rz_coord_axis = X               # Which axis the symmetry is around
[]

[Variables]
  [pressure]
    # Adds a Linear Lagrange variable by default
  []
[]

[Kernels]
  [diffusion]
    type = ADDiffusion  # Laplacian operator using automatic differentiation
    variable = pressure # Operate on the "pressure" variable from above
  []
[]

[BCs]
  [inlet]
    type = DirichletBC  # Simple u=value BC
    variable = pressure # Variable to be set
    boundary = left     # Name of a sideset in the mesh
    value = 4000        # (Pa) From Figure 2 from paper.  First data point for 1mm spheres.
  []
  [outlet]
    type = DirichletBC
    variable = pressure
    boundary = right
    value = 0           # (Pa) Gives the correct pressure drop from Figure 2 for 1mm spheres
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
