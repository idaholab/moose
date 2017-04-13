#
# Hoop stress
#
# This test checks that hoop stress is calculated correctly for the default orientation.
# It calculates the hoop stress for a hoop cenetered at (-25,0,0) with the default vector (0,1,0).
# The hoop has a radius = 20, t = 1.
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
  file = hoop_default_yaxis.e
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
  [./hoop2]
    order = CONSTANT
    family = MONOMIAL
    block = 2
  [../]
  [./radial2]
    order = CONSTANT
    family = MONOMIAL
    block = 2
  [../]
  [./axial2]
    order = CONSTANT
    family = MONOMIAL
    block = 2
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
  [./hoop2]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    scalar_type = HoopStress
    variable = hoop2
    block = 2
    execute_on = timestep_end
  [../]
  [./radial2]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    scalar_type = RadialStress
    variable = radial2
    block = 2
    execute_on = timestep_end
  [../]
  [./axial2]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    scalar_type = AxialStress
    variable = axial2
    block = 2
    execute_on = timestep_end
  [../]

[] # AuxKernels

[BCs]

  [./fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = '11'
    value = 0
  [../]
  [./fix_y]
    type = DirichletBC
    variable = disp_y
    boundary = '200'
    value = 0
  [../]
  [./fix_z]
    type = DirichletBC
    variable = disp_z
    boundary = '13'
    value = 0
  [../]

  [./Pressure]
    [./internal_pressure]
      disp_x = disp_x
      disp_y = disp_y
      disp_z = disp_z
      boundary = 1
      function = pressure
    [../]
  [../]

[] # BCs

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = 2
    youngs_modulus = 1e6
    poissons_ratio = 0.35
  [../]
  [./small_strain]
    type = ComputeIncrementalSmallStrain
    block = 2
  [../]
  [./elastic_stress]
    type = ComputeFiniteStrainElasticStress
    block = 2
  [../]
[]

[Executioner]

  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


  petsc_options = '-ksp_gmres_modifiedgramschmidt'
  petsc_options_iname = '-ksp_gmres_restart -pc_type  -pc_hypre_type'
  petsc_options_value = '201                 hypre     boomeramg     '


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
  file_base = hoop_stress_default_yaxis_out
[] # Outputs
