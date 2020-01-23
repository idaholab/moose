#
# This test checks elastic stress calculations with mechanical and thermal
# strain using small strain formulation. Young's modulus is 3600, and Poisson's ratio is 0.2.
# The axisymmetric, plane strain 1D mesh is pulled with 1e-6 strain.  Thus,
# the strain is [1e-6, 0, 1e-6] (xx, yy, zz).  This gives stress of
# [5e-3, 2e-3, 5e-3].  After a temperature increase of 100 with alpha of
# 1e-8, the stress becomes [-1e-3, -4e-3, -1e-3].
#

[GlobalParams]
  displacements = disp_x
[]

[Problem]
  coord_type = RZ
[]

[Mesh]
  file = line.e
[]

[Variables]
  [./disp_x]
  [../]
[]

[AuxVariables]
  [./temp]
    initial_condition = 580.0
  [../]
[]

[Functions]
  [./temp]
    type = PiecewiseLinear
    x = '0   1   2'
    y = '580 580 680'
  [../]
  [./disp_x]
    type = PiecewiseLinear
    x = '0 1'
    y = '0 2e-6'
  [../]
[]

[Modules]
  [./TensorMechanics]
    [./Master]
      [./ps]
        planar_formulation = PLANE_STRAIN
        strain = SMALL
        generate_output = 'strain_xx strain_zz stress_xx stress_yy stress_zz'
        eigenstrain_names = eigenstrain
      [../]
    [../]
  [../]
[]

[AuxKernels]
  [./temp]
    type = FunctionAux
    variable = temp
    function = temp
    execute_on = 'timestep_begin'
  [../]
[]

[BCs]
  [./no_x]
    type = DirichletBC
    boundary = 1
    value = 0
    variable = disp_x
  [../]
  [./disp_x]
    type = FunctionDirichletBC
    boundary = 2
    function = disp_x
    variable = disp_x
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 3600
    poissons_ratio = 0.2
  [../]

  [./thermal_strain]
    type = ComputeThermalExpansionEigenstrain
    thermal_expansion_coeff = 1e-8
    temperature = temp
    stress_free_temperature = 580
    eigenstrain_name = eigenstrain
  [../]

  [./stress]
    type = ComputeLinearElasticStress
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  line_search = 'none'

  l_max_its = 50
  l_tol = 1e-6
  nl_max_its = 15
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10
  start_time = 0
  end_time = 2
  num_steps = 2
[]

[Outputs]
  exodus = true
  console = true
[]
