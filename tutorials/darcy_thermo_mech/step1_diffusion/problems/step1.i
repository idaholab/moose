[Mesh]
  type = GeneratedMesh # Can generate simple lines, rectangles and rectangular prisms
  dim = 2 # Dimension of the mesh
  nx = 100 # Number of elements in the x direction
  ny = 10 # Number of elements in the y direction
  xmax = 0.304  # Length of test chamber
  ymax = 0.0257  # Test chamber radius
[]

[Variables]
  [./pressure]
    # Adds a Linear Lagrange variable by default
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion # A Laplacian operator
    variable = pressure # Operate on the "pressure" variable from above
  [../]
[]

[BCs]
  [./inlet]
    type = DirichletBC # Simple u=value BC
    variable = pressure
    boundary = left # Name of a sideset in the mesh
    value = 4000 # (Pa) From Figure 2 from paper.  First dot for 1mm balls.
  [../]
  [./outlet]
    type = DirichletBC
    variable = pressure
    boundary = right
    value = 0 # (Pa) Gives the correct pressure drop from Figure 2 for 1mm balls
  [../]
[]

[Executioner]
  type = Steady # Steady state problem
  solve_type = PJFNK #Preconditioned Jacobian Free Newton Krylov
  petsc_options_iname = '-pc_type -pc_hypre_type' #Matches with the values below
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  output_initial = true # Output initial condiiton
  exodus = true # Output Exodus format
  [./console]
    type = Console
    perf_log = true
    linear_residuals = true
  [../]
[]
