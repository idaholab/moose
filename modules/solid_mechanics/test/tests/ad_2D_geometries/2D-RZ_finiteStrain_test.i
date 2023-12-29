# Considers the mechanics solution for a thick spherical shell that is uniformly
# pressurized on the inner and outer surfaces, using 2D axisymmetric geometry.
# This test uses the strain calculator ComputeAxisymmetricRZFiniteStrain,
# which is generated through the use of the TensorMechanics MasterAction.
#
# From Roark (Formulas for Stress and Strain, McGraw-Hill, 1975), the radially-dependent
# circumferential stress in a uniformly pressurized thick spherical shell is given by:
#
# S(r) = [ Pi[ri^3(2r^3+ro^3)] - Po[ro^3(2r^3+ri^3)] ] / [2r^3(ro^3-ri^3)]
#
#   where:
#          Pi = inner pressure
#          Po = outer pressure
#          ri = inner radius
#          ro = outer radius
#
# The tests assume an inner and outer radii of 5 and 10, with internal and external
# pressures of 100000 and 200000 at t = 1.0, respectively. The resulting compressive
# tangential stress is largest at the inner wall and, from the above equation, has a
# value of -271429.
#
# RESULTS are below. Since stresses are average element values, values for the
# edge element and one-element-in are used to extrapolate the stress to the
# inner surface. The vesrion of the tests that are checked use the coarsest meshes.
#
#  Mesh    Radial elem   S(edge elem)  S(one elem in)  S(extrap to surf)
# 1D-SPH
# 2D-RZ        12 (x10)    -265004      -254665        -270174
#  3D          12 (6x6)    -261880      -252811        -266415
#
# 1D-SPH
# 2D-RZ        48 (x10)    -269853      -266710        -271425
#  3D          48 (10x10)  -268522      -265653        -269957
#
# The numerical solution converges to the analytical solution as the mesh is
# refined.

[Mesh]
  file = 2D-RZ_mesh.e
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
    block = 1
    use_automatic_differentiation = true
  [../]
[]

[AuxVariables]
  [./stress_theta]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_theta]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./stress_theta]
    type = ADRankTwoAux
    rank_two_tensor = stress
    index_i = 2
    index_j = 2
    variable = stress_theta
    execute_on = timestep_end
  [../]
  [./strain_theta]
    type = ADRankTwoAux
    rank_two_tensor = total_strain
    index_i = 2
    index_j = 2
    variable = strain_theta
    execute_on = timestep_end
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1e10
    poissons_ratio = 0.345
    block = 1
  [../]

  [./_elastic_strain]
    type = ADComputeFiniteStrainElasticStress
    block = 1
  [../]
[]

[BCs]
# pin particle along symmetry planes
  [./no_disp_r]
    type = ADDirichletBC
    variable = disp_r
    boundary = xzero
    value = 0.0
  [../]

  [./no_disp_z]
    type = ADDirichletBC
    variable = disp_z
    boundary = yzero
    value = 0.0
  [../]

# exterior and internal pressures
  [./exterior_pressure_r]
    type = ADPressure
    variable = disp_r
    boundary = outer
    component = 0
    function = '200000*t'
  [../]

 [./exterior_pressure_z]
    type = ADPressure
    variable = disp_z
    boundary = outer
    component = 1
    function = '200000*t'
  [../]

  [./interior_pressure_r]
    type = ADPressure
    variable = disp_r
    boundary = inner
    component = 0
    function = '100000*t'
  [../]

  [./interior_pressure_z]
    type = ADPressure
    variable = disp_z
    boundary = inner
    component = 1
    function = '100000*t'
  [../]
[]

[Debug]
  show_var_residual_norms = true
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
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
  end_time = 0.2
  dt = 0.1
[]

[Postprocessors]
  [./strainTheta]
    type = ElementAverageValue
    variable = strain_theta
  [../]
  [./stressTheta]
    type = ElementAverageValue
    variable = stress_theta
  [../]
  [./stressTheta_pt]
    type = PointValue
    point = '5.0 0.0 0.0'
    #bottom inside edge for comparison to theory; use csv = true
    variable = stress_theta
  [../]
[]

[Outputs]
  exodus = true
[]
