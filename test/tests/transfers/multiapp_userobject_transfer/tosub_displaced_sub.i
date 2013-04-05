[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 8
  xmax = 0.1
  ymax = 0.5
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./u]
    initial_condition = 1
  [../]
[]

[AuxVariables]
  [./multi_layered_average]
  [../]
  [./element_multi_layered_average]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[Functions]
  [./axial_force]
    type = ParsedFunction
    value = 1000*y
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./td]
    type = TimeDerivative
    variable = u
  [../]
  [./force]
    type = UserForcingFunction
    variable = u
    function = axial_force
  [../]
[]

[AuxKernels]
  [./disp_x]
    type = ConstantAux
    variable = disp_x
    execute_on = initial
  [../]
  [./disp_y]
    type = ConstantAux
    variable = disp_y
    value = 0.5
    execute_on = initial
  [../]
[]

[BCs]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 0.001
  petsc_options = -snes_mf_operator
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Output]
  output_initial = true
  exodus = true
  perf_log = true
[]

[Problem]
  coord_type = rz
  type = FEProblem
[]

