#
# This problem is modified from the Abaqus verification manual:
#   "1.5.4 Patch test for axisymmetric elements"
# The original stress solution is given as:
#   xx = yy = zz = 2000
#   xy = 400
#
# Here, E=1e6 and nu=0.25.
# However, with a +100 degree change in temperature and a coefficient
#   of thermal expansion of 1e-6, the solution becomes:
#   xx = yy = zz = 1800
#   xy = 400
#   since
#   E*(1-nu)/(1+nu)/(1-2*nu)*(1+2*nu/(1-nu))*(1e-3-1e-4) = 1800
#
# Also,
#
#   dSrr   dSrz   Srr-Stt
#   ---- + ---- + ------- + br = 0
#    dr     dz       r
#
# and
#
#   dSrz   Srz   dSzz
#   ---- + --- + ---- + bz = 0
#    dr     r     dz
#
# where
#   Srr = stress in rr
#   Szz = stress in zz
#   Stt = stress in theta-theta
#   Srz = stress in rz
#   br  = body force in r direction
#   bz  = body force in z direction
#
# This test is meant to exercise the Jacobian.  To that end, the body
# force has been turned off.  This makes the results differ slightly
# from the original values, but requires a correct Jacobian for minimal
# iterations.  Iteration plotting is turned on to ensure that the
# number of iterations needed does not increase.

[Problem]
  coord_type = RZ
[]

[Mesh]
  file = elastic_thermal_patch_rz_test.e
[]

[Functions]
  [./ur]
    type = ParsedFunction
    value = '1e-3*x'
  [../]
  [./uz]
    type = ParsedFunction
    value = '1e-3*(x+y)'
  [../]
  [./body]
    type = ParsedFunction
    value = '-400/x'
  [../]
  [./temp]
    type = ParsedFunction
    value = '117.56+100*t'
  [../]
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]

  [./temp]
    initial_condition = 117.56
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
[]

[SolidMechanics]
  [./solid]
    disp_r = disp_x
    disp_z = disp_y
    temp = temp
  [../]
[]

[Kernels]
  [./heat]
    type = HeatConduction
    variable = temp
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
  [./stress_xy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xy
    index = 3
    execute_on = timestep_end
  [../]
  [./stress_yz]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yz
    index = 4
    execute_on = timestep_end
  [../]
  [./stress_zx]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_zx
    index = 5
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./ur]
    type = FunctionPresetBC
    variable = disp_x
    boundary = 10
    function = ur
  [../]
  [./uz]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 10
    function = uz
  [../]

  [./temp]
    type = FunctionPresetBC
    variable = temp
    boundary = 10
    function = temp
  [../]
[]

[Materials]
  [./stiffStuff1]
    type = Elastic
    block = 1

    disp_r = disp_x
    disp_z = disp_y

    bulk_modulus = 666666.6666666667
    poissons_ratio = 0.25

    temp = temp
    thermal_expansion = 1e-6
  [../]

  [./heat]
    type = HeatConductionMaterial
    block = 1

    specific_heat = 0.116
    thermal_conductivity = 4.85e-4
  [../]

  [./density]
    type = Density
    block = 1
    density = 0.283
    disp_r = disp_x
    disp_z = disp_y
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  nl_abs_tol = 1e-11
  nl_rel_tol = 1e-12

  l_max_its = 20

  start_time = 0.0
  dt = 1.0
  num_steps = 1
  end_time = 1.0
[]

[Outputs]
  file_base = elastic_thermal_patch_rz_smp_out
  [./exodus]
    type = Exodus
    elemental_as_nodal = true
    execute_on = 'initial timestep_end nonlinear'
    nonlinear_residual_dt_divisor = 100
  [../]
[]
