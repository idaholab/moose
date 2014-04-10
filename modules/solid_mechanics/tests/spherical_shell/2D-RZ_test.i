# Considers the mechanics solution for a thick spherical shell that is uniformly
# pressurized on the inner and outer surfaces. Three versions of the test are
# provided, assuming 1D spherical geometry (1D-SPH), 2D axisymmetic geometry
# (2D-RZ), and 3D geometry (3D). The tests demonstrate that all three geometric
# approaches produce the same correct solution.
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
# pressures of 100 and 200, respectively. The resulting compressive tangential
# stress is largest at the inner wall and, from the above equation, has a value
# of -271.429.
#
# RESULTS are below. Since stresses are average element values, values for the
# edge element and one-element-in are used to extrapolate the stress to the
# inner surface. The vesrion of the tests that are checked use the coarsest meshes.
#
#  Mesh    Radial elem   S(edge elem)  S(one elem in)  S(extrap to surf)
# 1D-SPH       12          -264.842      -254.419        -270.053
# 2D-RZ        12          -265.007      -254.668        -270.177
#  3D          12 (4x4)    -258.922      -251.099        -262.834
#  3D          12 (6x6)    -262.194      -252.969        -266.807
#
# 1D-SPH       48          -269.679      -266.390        -271.323
# 2D-RZ        48          -269.723      -266.470        -271.350
#  3D          48          -268.617      -265.717        -270.067
#
# 1D-SPH      100          -270.580      -268.932        -271.404
# 2D-RZ       100          -270.587      -268.946        -271.408
#
# The numerical solution converges to the analytical solution as the mesh is
# refined.

[GlobalParams]
  order = SECOND
  family = LAGRANGE
  disp_x = disp_x
  disp_y = disp_y
[]

[Mesh]
  file = 2D-RZ_mesh.e
  displacements = 'disp_x disp_y'
[]

[Problem]
  coord_type = RZ
[]

[Variables]
  [./disp_x]
  [../]

  [./disp_y]
  [../]
[]


[AuxVariables]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_r = disp_x
    disp_z = disp_y
  [../]
[]

[AuxKernels]
  [./stress_zz]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_zz
    index = 2
    execute_on = timestep
  [../]
[]

[BCs]

# pin particle along symmetry planes
  [./no_disp_x]
    type = DirichletBC
    variable = disp_x
    boundary = xzero
    value = 0.0
  [../]

  [./no_disp_y]
    type = DirichletBC
    variable = disp_y
    boundary = yzero
    value = 0.0
  [../]

# exterior and internal pressures
  [./exterior_pressure_x]
    type = Pressure
    variable = disp_x
    boundary = outer
    component = 0
    factor = 200
  [../]

 [./exterior_pressure_y]
    type = Pressure
    variable = disp_y
    boundary = outer
    component = 1
    factor = 200
  [../]

  [./interior_pressure_x]
    type = Pressure
    variable = disp_x
    boundary = inner
    component = 0
    factor = 100
  [../]

  [./interior_pressure_y]
    type = Pressure
    variable = disp_y
    boundary = inner
    component = 1
    factor = 100
  [../]
[]

[Materials]

 [./fuel_disp]                         # thermal and irradiation creep for UO2 (bison kernel)
    type = Elastic
    block = 1
    disp_r = disp_x
    disp_z = disp_y
    youngs_modulus = 1e10
    poissons_ratio = .345
    thermal_expansion = 0
  [../]
[]

[Debug]
    show_var_residual_norms = true
[]

[Executioner]
  type = Transient

  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      4'

  line_search = 'none'

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  nl_rel_tol = 5e-6
  nl_abs_tol = 1e-10
  nl_max_its = 15

  l_tol = 1e-3
  l_max_its = 50

   start_time = 0.0
   end_time = 1
   num_steps = 1000

  dtmax = 5e6
  dtmin = 1
  [./TimeStepper]
    type = IterationAdaptiveDT
    dt = 1
    optimal_iterations = 6
    iteration_window = 0.4
    linear_iteration_ratio = 100
  [../]

  [./Predictor]
    type = SimplePredictor
    scale = 1.0
  [../]

#  [./Quadrature]
#    order = THIRD
#  [../]
[]

[Postprocessors]
  [./dt]
    type = TimestepSize
  [../]
[]

[Outputs]
  file_base = 2D-RZ_out
  output_initial = true
  exodus = true
  [./console]
    type = Console
    perf_log = true
    linear_residuals = true
  [../]
[]
