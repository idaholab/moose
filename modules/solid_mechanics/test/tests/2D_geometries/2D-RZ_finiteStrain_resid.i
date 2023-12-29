# This tests the save_in_disp residual aux-variables for
# ComputeAxisymmetricRZFiniteStrain, which is generated through the use of the
# TensorMechanics MasterAction. The GeneratedMesh is 1x1, rotated via axisym to
# create a cylinder of height 1, radius 1.
#
# PostProcessor force_z plots the force on the top surface of the cylinder.
#
# Displacement of 0.1 is applied to top of cylinder while other surfaces are
# constrained. Plotting force_z vs stress_z will show a slope of 3.14159 (pi),
# consistent with formula for normal stress:
#
# Stress = force / area
#
# where area is A = pi * r^2 for a circle.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 1.0
[]

[GlobalParams]
  displacements = 'disp_r disp_z'
[]

[Problem]
  coord_type = RZ
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    add_variables = true
    save_in = 'force_r force_z'
  [../]
[]

[AuxVariables]
  [./stress_r]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_r]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_z]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_z]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./force_r]
    order = FIRST
    family = LAGRANGE
  [../]
  [./force_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxKernels]
  [./stress_r]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 0
    index_j = 0
    variable = stress_r
    execute_on = timestep_end
  [../]
  [./strain_r]
    type = RankTwoAux
    rank_two_tensor = total_strain
    index_i = 0
    index_j = 0
    variable = strain_r
    execute_on = timestep_end
  [../]
  [./stress_z]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 1
    index_j = 1
    variable = stress_z
    execute_on = timestep_end
  [../]
  [./strain_z]
    type = RankTwoAux
    rank_two_tensor = total_strain
    index_i = 1
    index_j = 1
    variable = strain_z
    execute_on = timestep_end
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]

  [./_elastic_strain]
    type = ComputeFiniteStrainElasticStress
  [../]
[]

[BCs]
  [./no_disp_r_left]
    type = DirichletBC
    variable = disp_r
    boundary = left
    value = 0.0
  [../]
  [./no_disp_r_right]
    type = DirichletBC
    variable = disp_r
    boundary = right
    value = 0.0
  [../]
  [./no_disp_z_bottom]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0.0
  [../]
  [./top]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = top
    function = 't'
  [../]
[]

[Debug]
    show_var_residual_norms = true
[]

[Executioner]
  type = Transient

  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '  201               hypre    boomeramg      10'

  line_search = 'none'

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  nl_rel_tol = 5e-9
  nl_abs_tol = 1e-10
  nl_max_its = 15

  l_tol = 1e-3
  l_max_its = 50

  start_time = 0.0
  end_time = 0.1
  dt = 0.01
[]

[Postprocessors]
  [./strainR]
    type = ElementAverageValue
    variable = strain_r
  [../]
  [./stressR]
    type = ElementAverageValue
    variable = stress_r
  [../]
  [./strainZ]
    type = ElementAverageValue
    variable = strain_z
  [../]
  [./stressZ]
    type = ElementAverageValue
    variable = stress_z
  [../]
  [./force_r]
    type = NodalSum
    variable = force_r
    boundary = top
  [../]
  [./force_z]
    type = NodalSum
    variable = force_z
    boundary = top
  [../]
[]

[Outputs]
  exodus = true
  #csv = true
  print_linear_residuals = false
  perf_graph = true
[]
