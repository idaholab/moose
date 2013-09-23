[Mesh]
  type = FileMesh
  file = twoTrussLine.e
[]

[Variables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./axial_stress]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e_over_l]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./area]
    order = CONSTANT
    family = MONOMIAL
#    initial_condition = 1.0
  [../]
[]

[Functions]
  [./x2]
    type = PiecewiseLinear
    x = '0  1 2 3'
    y = '0 .5 1 1'
  [../]
  [./y2]
    type = PiecewiseLinear
    x = '0 1  2 3'
    y = '0 0 .5 1'
  [../]
[]

[BCs]
  [./fixx1]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./fixx2]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 2
    function = x2
  [../]
  [./fixx3]
    type = DirichletBC
    variable = disp_x
    boundary = 3
    value = 0.0
  [../]
  [./fixy1]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0
  [../]
  [./fixy2]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 2
    function = y2
  [../]
  [./fixy3]
    type = DirichletBC
    variable = disp_y
    boundary = 3
    value = 0
  [../]
  [./fixz1]
    type = DirichletBC
    variable = disp_z
    boundary = 1
    value = 0
  [../]
  [./fixz2]
    type = DirichletBC
    variable = disp_z
    boundary = 2
    value = 0
  [../]
  [./fixz3]
    type = DirichletBC
    variable = disp_z
    boundary = 3
    value = 0
  [../]
[]

[AuxKernels]
  [./axial_stress]
    type = MaterialRealAux
    property = axial_stress
    variable = axial_stress
  [../]
  [./e_over_l]
    type = MaterialRealAux
    property = e_over_l
    variable = e_over_l
  [../]
  [./area]
    type = ConstantAux
    variable = area
    value = 1.0
    execute_on = timestep_begin
  [../]
[]

[Executioner]
  type = Transient
  petsc_options = '-snes_mf_operator -ksp_monitor'
  petsc_options_iname = '-pc_type -snes_type -snes_ls -snes_linesearch_type -ksp_gmres_restart'
  petsc_options_value = 'jacobi   ls         basic    basic                    101'

  nl_max_its = 15
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-10

  dt = 1
  num_steps = 3
  end_time = 3
[]

[Kernels]
  [./solid_x]
    type = StressDivergenceTruss
    variable = disp_x
    component = 0
    area = area
  [../]
  [./solid_y]
    type = StressDivergenceTruss
    variable = disp_y
    component = 0
    area = area
  [../]
  [./solid_z]
    type = StressDivergenceTruss
    variable = disp_z
    component = 0
    area = area
  [../]
[]

#[SolidMechanics]
#  [./solid]
#    type = truss
#    disp_x = disp_x
#    disp_y = disp_y
#    disp_z = disp_z
#    area = area
#  [../]
#[]

[Materials]
  [./steel]
    type = HeatConductionMaterial
    block = '1 2'
    thermal_conductivity = 10
    specific_heat = 1
  [../]
  [./linelast]
    type = TrussMaterial
    block = '1 2'
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    youngs_modulus = 1e6
#    thermal_expansion = 0.1
#    t_ref = 0.5
#    temp = temp
  [../]
  [./density]
    type = Density
    block = '1 2'
    density = 1
  [../]
[]

[Output]
  file_base = solid_mech_truss_out
  output_initial = true
  exodus = true
  perf_log = true
[]
