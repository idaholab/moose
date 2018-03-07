[GlobalParams]
  order = FIRST
  family = LAGRANGE
[]

[XFEM]
  geometric_cut_userobjects = 'cut_mesh'
  output_cut_plane = true
  qrule = volfrac
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 5
  ny = 5
  nz = 5
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 1.0
  zmin = -0.4
  zmax = 0.6
  elem_type = HEX8
  displacements = 'disp_x disp_y disp_z'
[]

[UserObjects]
  [./cut_mesh]
    type = MeshCut3DUserObject
    mesh_file = mesh_grow.xda
    function_x = growth_func_x
    function_y = growth_func_y
    function_z = growth_func_z
# The current gold file does not grow the cutting mesh, but this is something
# that needs to be tested more in the future.
#    size_control = 0.05
#    n_step_growth = 50
  [../]
[]

[Functions]
  [./growth_func_x]
    type = ParsedFunction
    value = 5*(x-0.3)+z
  [../]
  [./growth_func_y]
    type = ParsedFunction
    value = 5*(y-0.5)+(z+x)/2
  [../]
  [./growth_func_z]
    type = ParsedFunction
    value = 5*(z-0.1)+x
  [../]
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
  []

[AuxVariables]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./vonmises]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./SED]
   order = CONSTANT
    family = MONOMIAL
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    use_displaced_mesh = true
  [../]
[]

[AuxKernels]
  [./stress_xx]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xx
    index = 0
    execute_on = timestep_end
  [../]
  [./stress_yy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yy
    index = 1
    execute_on = timestep_end
  [../]
  [./stress_zz]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_zz
    index = 2
    execute_on = timestep_end
  [../]
  [./vonmises]
    type = MaterialTensorAux
    tensor = stress
    variable = vonmises
    quantity = vonmises
    execute_on = timestep_end
  [../]
  [./SED]
    type = MaterialRealAux
    variable = SED
    property = strain_energy_density
    execute_on = timestep_end
    block = 0
  [../]
[]

[Functions]
  [./top_trac_y]
    type = ConstantFunction
    value = 10
  [../]
[]


[BCs]
  [./top_y]
    type = FunctionNeumannBC
    boundary = top
    variable = disp_y
    function = top_trac_y
  [../]
  [./bottom_x]
    type = PresetBC
    boundary = bottom
    variable = disp_x
    value = 0.0
  [../]
  [./bottom_y]
    type = PresetBC
    boundary = bottom
    variable = disp_y
    value = 0.0
  [../]
  [./bottom_z]
    type = PresetBC
    boundary = bottom
    variable = disp_z
    value = 0.0
  [../]
[]

[Materials]
  [./linelast]
    type = Elastic
    block = 0
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    poissons_ratio = 0.3
    youngs_modulus = 207000
    compute_JIntegral = true
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      8'

  line_search = 'none'

  [./Predictor]
    type = SimplePredictor
    scale = 1.0
  [../]

# controls for linear iterations
  l_max_its = 100
  l_tol = 1e-2

# controls for nonlinear iterations
  nl_max_its = 15
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-10

# time control
  start_time = 0.0
  dt = 1.0
  end_time = 1.0
[]

[Outputs]
  file_base = mesh_grow
  execute_on = 'timestep_end'
  exodus = true
  [./console]
    type = Console
    perf_log = true
    output_linear = true
  [../]
[]
