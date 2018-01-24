#This problem from [Wilson 1979] tests the thermal strain term in the
#interaction integral
#
#theta_e = 10 degrees C; a = 252; E = 207000; nu = 0.3; alpha = 1.35e-5
#
#With uniform_refine = 3, KI converges to
#KI = 5.602461e+02 (interaction integral)
#KI = 5.655005e+02 (J-integral)
#
#Both are in good agreement with [Shih 1986]:
#average_value = 0.4857 = KI / (sigma_theta * sqrt(pi * a))
#sigma_theta = E * alpha * theta_e / (1 - nu)
# = 207000 * 1.35e-5 * 10 / (1 - 0.3) = 39.9214
#KI = average_value * sigma_theta * sqrt(pi * a) = 5.656e+02
#
#References:
#W.K. Wilson, I.-W. Yu, Int J Fract 15 (1979) 377-387
#C.F. Shih, B. Moran, T. Nakamura, Int J Fract 30 (1986) 79-102

[GlobalParams]
  order = FIRST
  family = LAGRANGE
  disp_x = disp_x
  disp_y = disp_y
  volumetric_locking_correction = False
  displacements = 'disp_x disp_y'
[]

[Mesh]
  file = crack2d.e
#  uniform_refine = 3
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]


[AuxVariables]
  [./stress_xx]      # stress aux variables are defined for output; this is a way to get integration point variables to the output file
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
  [./vonmises]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./SED]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./temp]
    order = FIRST
    family = LAGRANGE
  [../]
[]


[Functions]
  [./tempfunc]
    type = ParsedFunction
    value = 10.0*(2*x/504)
  [../]
[]

[DomainIntegral]
  integrals = 'JIntegral InteractionIntegralKI'
  boundary = 800
  crack_direction_method = CrackDirectionVector
  crack_direction_vector = '1 0 0'
  2d = true
  axis_2d = 2
  radius_inner = '60.0 80.0 100.0 120.0'
  radius_outer = '80.0 100.0 120.0 140.0'
  convert_J_to_K = true
  symmetry_plane = 1

  # interaction integral parameters
  disp_x = disp_x
  disp_y = disp_y
  block = 1
  youngs_modulus = 207000
  poissons_ratio = 0.3
  temp = temp
  solid_mechanics = true
[]

[SolidMechanics]
  [./solid]
  [../]
[]

[AuxKernels]
  [./stress_xx]               # computes stress components for output
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xx
    index = 0
    execute_on = timestep_end     # for efficiency, only compute at the end of a timestep
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
  [./vonmises]
    type = MaterialTensorAux
    tensor = stress
    variable = vonmises
    quantity = vonmises
    execute_on = timestep_end
  [../]
  [./SED]
    type = MaterialRealAux
    variable = SED
    property = strain_energy_density
    execute_on = timestep_end
  [../]
  [./tempfuncaux]
    type = FunctionAux
    variable = temp
    function = tempfunc
    block = 1
  [../]
[]

[BCs]

  [./crack_y]
    type = PresetBC
    variable = disp_y
    boundary = 100
    value = 0.0
  [../]

  [./no_y]
    type = PresetBC
    variable = disp_y
    boundary = 400
    value = 0.0
  [../]

  [./no_x1]
    type = PresetBC
    variable = disp_x
    boundary = 900
    value = 0.0
  [../]

[] # BCs

[Materials]
  [./stiffStuff]
    type = Elastic
    block = 1

    disp_x = disp_x
    disp_y = disp_y

    youngs_modulus = 207000
    poissons_ratio = 0.3
    thermal_expansion = 1.35e-5
    formulation = NonlinearPlaneStrain
    compute_JIntegral = true
    compute_InteractionIntegral = true
    temp = temp
    stress_free_temperature = 0.0
  [../]
[]


[Executioner]

  type = Transient

  solve_type = 'PJFNK'

  petsc_options = ksp_monitor
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_ksp_type -sub_pc_type -pc_asm_overlap'
  petsc_options_value = 'asm         31   preonly   lu      1'

  line_search = 'none'

   l_max_its = 50
   nl_max_its = 40

   nl_rel_step_tol= 1e-10
   nl_rel_tol = 1e-10


   start_time = 0.0
   dt = 1

   end_time = 1
   num_steps = 1

[]

[Outputs]
  file_base = interaction_integral_2d_out
  exodus = true
  csv = true
[]

[Preconditioning]
  active = 'smp'
  [./smp]
    type = SMP
    pc_side = left
    ksp_norm = preconditioned
    full = true
  [../]
[]
