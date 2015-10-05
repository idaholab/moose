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

[Mesh]#Comment
  file = elastic_thermal_patch_rz_test.e
[] # Mesh

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
[] # Functions

[Variables]

  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]

  [./temp]
    order = FIRST
    family = LAGRANGE
    initial_condition = 117.56
  [../]

[] # Variables

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

[] # AuxVariables

[SolidMechanics]
  [./solid]
    disp_r = disp_x
    disp_z = disp_y
    temp = temp
  [../]
[]

[Kernels]

# Turned off to more fully test the full Jacobian
#  [./body]
#    type = BodyForce
#    variable = disp_y
#    value = 1
#    function = body
#  [../]

  [./heat]
    type = HeatConduction
    variable = temp
  [../]

[] # Kernels

[AuxKernels]

  [./stress_xx]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xx
    index = 0
  [../]
  [./stress_yy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yy
    index = 1
  [../]
  [./stress_zz]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_zz
    index = 2
  [../]
  [./stress_xy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xy
    index = 3
  [../]
  [./stress_yz]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yz
    index = 4
  [../]
  [./stress_zx]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_zx
    index = 5
  [../]

[] # AuxKernels

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

[] # BCs

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

[] # Materials

[Preconditioning]
#  [./FDP]
#    type = FDP
#    full = true
#  [../]

  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]

  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'



  petsc_options_iname = '-pc_type -ksp_gmres_restart'
  petsc_options_value = 'lu       101'


  line_search = 'none'


  nl_abs_tol = 1e-11
  nl_rel_tol = 1e-12


  l_max_its = 20

  start_time = 0.0
  dt = 1.0
  num_steps = 1
  end_time = 1.0
[] # Executioner

[Outputs]
  file_base = out_rz_smp
  [./exodus]
    type = Exodus
    elemental_as_nodal = true
    execute_on = 'initial timestep_end nonlinear'
    nonlinear_residual_dt_divisor = 100
  [../]
[] # Outputs
