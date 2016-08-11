# This problem is intended to exercise the Jacobian for coupled RZ
# problems.  Only two iterations should be needed.

[Problem]
  coord_type = RZ
[]


[Mesh]#Comment
  file = elastic_thermal_patch_rz_test.e
[] # Mesh

[Functions]
  [./ur]
    type = ParsedFunction
    value = '0'
  [../]
  [./uz]
    type = ParsedFunction
    value = '0'
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
    boundary = 1
    function = ur
  [../]
  [./uz]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 2
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

    youngs_modulus = 1e6
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


  nl_abs_tol = 1e-9
  nl_rel_tol = 1e-12


  l_max_its = 20

  start_time = 0.0
  dt = 1.0
  num_steps = 1
  end_time = 1.0
[] # Executioner

[Outputs]
  file_base = out_jac_rz_smp
  [./exodus]
    type = Exodus
    elemental_as_nodal = true
    execute_on = 'initial timestep_end nonlinear'
    nonlinear_residual_dt_divisor = 100
  [../]
[] # Outputs
