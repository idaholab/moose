#
# Hoop stress
#
# This test checks that hoop stress is calculated correctly for three orientations.
# 1. A hoop centered at (20,20,20) with vector (0,0,1)
# 2. A hoop centered at (-25,20,20) with vector (0,1,0)
# 3. A hoop centered at (-20,-20,20) with vector (1,0,0)
# All three have radius = 20, t = 1.
#
# Hoop stress should be P*r/t -> 1e3*20/1 = 20e3
#
# The output hoop stress is close to this value (nonlinear geometry is on) for all
# elements.
#

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  order = FIRST
  family = LAGRANGE
[]

[Mesh]
  file = hoops.e
[]

[Functions]
  [./pressure]
    type = PiecewiseLinear
    x = '0. 1.'
    y = '0. 1.'
    scale_factor = 1e3
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
  [./stress_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./hoop1]
    order = CONSTANT
    family = MONOMIAL
    block = 1
  [../]
  [./hoop2]
    order = CONSTANT
    family = MONOMIAL
    block = 2
  [../]
  [./hoop3]
    order = CONSTANT
    family = MONOMIAL
    block = 3
  [../]
  [./radial1]
    order = CONSTANT
    family = MONOMIAL
    block = 1
  [../]
  [./radial2]
    order = CONSTANT
    family = MONOMIAL
    block = 2
  [../]
  [./radial3]
    order = CONSTANT
    family = MONOMIAL
    block = 3
  [../]
  [./axial1]
    order = CONSTANT
    family = MONOMIAL
    block = 1
  [../]
  [./axial2]
    order = CONSTANT
    family = MONOMIAL
    block = 2
  [../]
  [./axial3]
    order = CONSTANT
    family = MONOMIAL
    block = 3
  [../]

[] # AuxVariables

[Kernels]
  [./TensorMechanics]
    use_displaced_mesh = true
  [../]
[]

[AuxKernels]
  [./stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
    execute_on = timestep_end
  [../]
  [./stress_xy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xy
    index_i = 0
    index_j = 1
    execute_on = timestep_end
  [../]
  [./stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
    execute_on = timestep_end
  [../]
  [./stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 2
    index_j = 2
    execute_on = timestep_end
  [../]
  [./stress_yz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yz
    index_i = 1
    index_j = 2
    execute_on = timestep_end
  [../]
  [./stress_zx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zx
    index_i = 2
    index_j = 0
    execute_on = timestep_end
  [../]
  [./hoop1]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    scalar_type = HoopStress
    variable = hoop1
    block = 1
    point1 = '20 20 -4'
    point2 = '20 20 47'
    execute_on = timestep_end
  [../]
  [./hoop2]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    scalar_type = HoopStress
    variable = hoop2
    block = 2
    point1 = '-25 12 20'
    point2 = '-25 10 20'
    execute_on = timestep_end
  [../]
  [./hoop3]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    scalar_type = HoopStress
    variable = hoop3
    block = 3
    point1 = '0 -20 20'
    point2 = '16 -20 20'
    execute_on = timestep_end
  [../]
  [./radial1]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    scalar_type = RadialStress
    variable = radial1
    block = 1
    point1 = '20 20 -4'
    point2 = '20 20 47'
    execute_on = timestep_end
  [../]
  [./radial2]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    scalar_type = RadialStress
    variable = radial2
    block = 2
    point1 = '-25 12 20'
    point2 = '-25 10 20'
    execute_on = timestep_end
  [../]
  [./radial3]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    scalar_type = RadialStress
    variable = radial3
    block = 3
    point1 = '0 -20 20'
    point2 = '16 -20 20'
    execute_on = timestep_end
  [../]
  [./axial1]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    scalar_type = AxialStress
    variable = axial1
    block = 1
    point1 = '20 20 -4'
    point2 = '20 20 47'
    execute_on = timestep_end
  [../]
  [./axial2]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    scalar_type = AxialStress
    variable = axial2
    block = 2
    point1 = '-25 12 20'
    point2 = '-25 10 20'
    execute_on = timestep_end
  [../]
  [./axial3]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    scalar_type = AxialStress
    variable = axial3
    block = 3
    point1 = '0 -20 20'
    point2 = '16 -20 20'
    execute_on = timestep_end
  [../]

[] # AuxKernels

[BCs]

  [./fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = '300 11'
    value = 0
  [../]
  [./fix_y]
    type = DirichletBC
    variable = disp_y
    boundary = '200 12'
    value = 0
  [../]
  [./fix_z]
    type = DirichletBC
    variable = disp_z
    boundary = '100 13'
    value = 0
  [../]

  [./Pressure]
    [./internal_pressure]
      boundary = 1
      function = pressure
      disp_x = disp_x
      disp_y = disp_y
      disp_z = disp_z
    [../]
  [../]

[] # BCs

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.35
    block = '1 2 3'
  [../]
  [./small_strain]
    type = ComputeIncrementalSmallStrain
    block = '1 2 3'
  [../]
  [./elastic_stress]
    type = ComputeFiniteStrainElasticStress
    block = '1 2 3'
  [../]
[] # Materials

[Executioner]

  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


  petsc_options = '-snes_ksp_ew -ksp_gmres_modifiedgramschmidt'
  petsc_options_iname = '-ksp_gmres_restart -pc_type  -pc_hypre_type'
  petsc_options_value = '201                 hypre     boomeramg'


  line_search = 'none'

  l_tol = 1e-8
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-14

  l_max_its = 20

  start_time = 0.0
  dt = 1.0
  num_steps = 1
  end_time = 1.0
[] # Executioner

[Outputs]
  exodus = true
  file_base = hoop_stress_out
[] # Outputs
