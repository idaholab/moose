[Mesh]
 type = GeneratedMesh
 dim = 2
 xmin = 0
 xmax = 100
 ymin = 0
 ymax = 100
 elem_type = QUAD4
 nx = 64
 ny = 64

 uniform_refine = 0

 displacements = 'x_disp y_disp'
[]

#The minimum eigenvalue for this problem is 2*(pi/a)^2 + 2 with a = 100.
#Its inverse will be 0.49950700634518.

[Variables]
  [./u]
    # second order is way better than first order
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./x_disp]
  [../]
  [./y_disp]
  [../]
[]

[AuxKernels]
  [./x_disp]
    type = FunctionAux
    variable = x_disp
    function = x_disp_func
  [../]
  [./y_disp]
    type = FunctionAux
    variable = y_disp
    function = y_disp_func
  [../]
[]

[Functions]
  [./x_disp_func]
    type = ParsedFunction
    value = 0
  [../]
  [./y_disp_func]
    type = ParsedFunction
    value = 0
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
    use_displaced_mesh = true
  [../]

  [./rea]
    type = CoefReaction
    variable = u
    coefficient = 2.0
    use_displaced_mesh = true
  [../]

  [./rhs]
    type = Reaction
    variable = u
    use_displaced_mesh = true
    eigen_kernel = true
  [../]
[]

[BCs]
  [./homogeneous]
    type = DirichletBC
    variable = u
    boundary = '0 1 2 3'
    value = 0
    use_displaced_mesh = true
  [../]
[]

[Problem]
  type = EigenProblem
  eigen_problem_type = GNHEP
  n_eigen_pairs = 1
  n_basis_vectors = 18
[]

[Executioner]
  type = Steady
  eigen_solve_type = jd
  petsc_options = '-eps_smallest_magnitude -eps_view -eps_monitor_conv -eps_monitor'
[]

[VectorPostprocessors]
  [./eigenvalues]
    type = Eigenvalues
    execute_on = 'timestep_end'
  [../]
[]

[Outputs]
  csv = true
  execute_on = 'timestep_end'
  [./console]
    type = Console
    outlier_variable_norms = false
  [../]
[]
