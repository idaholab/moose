[Mesh]
  type = GeneratedMesh # Can generate simple lines, rectangles and rectangular prisms
  dim = 2 # Dimension of the mesh
  nx = 10 # Number of elements in the x direction
  ny = 10 # Number of elements in the y direction
  xmax = 1.0
  ymax = 1.0
[]

[Variables]
  [./T]
  [../]
[]

[AuxVariables]
  [./flux_x]
      order = FIRST
      family = MONOMIAL
  [../]
  [./flux_y]
      order = FIRST
      family = MONOMIAL
  [../]
[]

[Kernels]
  active = 'diff'
  [./diff]
    type = MatDiffusionTest # A Laplacian operator
    variable = T
    prop_name = 'thermal_conductivity'
  [../]
  [./diff_ad]
    type = ADMatDiffusion # A Laplacian operator
    variable = T
    diffusivity = 'thermal_conductivity'
  [../]
[]

[AuxKernels]
  [./flux_x]
    type = DiffusionFluxAux
    diffusivity = 'thermal_conductivity'
    variable = flux_x
    diffusion_variable = T
    component = x
  [../]
  [./flux_y]
    type = DiffusionFluxAux
    diffusivity = 'thermal_conductivity'
    variable = flux_y
    diffusion_variable = T
    component = y
  [../]
[]

[BCs]
  [./inlet]
    type = DirichletBC # Simple u=value BC
    variable = T
    boundary = left
    value = 4000 # K
  [../]
  [./outlet]
    type = DirichletBC
    variable = T
    boundary = right
    value = 400 # K
  [../]
[]

[Materials]
  [./k]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '10' # in W/mK
  [../]
[]

[VectorPostprocessors]
  # avoid sampling an element variable on faces
  [./line_sample]
    type = LineValueSampler
    variable = 'T flux_x flux_y'
    start_point = '0.01 0.01 0'
    end_point = '0.98 0.01 0'
    num_points = 11
    sort_by = id
  [../]
[]

[Executioner]
  type = Steady # Steady state problem
  solve_type = PJFNK #Preconditioned Jacobian Free Newton Krylov
  nl_rel_tol = 1e-12
  petsc_options_iname = '-pc_type -pc_hypre_type' #Matches with the values below
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true # Output Exodus format
  execute_on = 'initial timestep_end'
[]
