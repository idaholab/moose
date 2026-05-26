# Simple 3D test -- same kinematics + plasticity as `new_system_manual_plasticity.i` but driven
# through the QuasiStatic Physics action's `compatibility_mode = true` shim. The action sees an
# OLD-style block (strain = FINITE, volumetric_locking_correction = true, decomposition_method)
# and configures the Lagrangian kernels + ComputeLagrangianWrappedStress + the right
# `kinematic_approximation`/`F_bar_mode`/`publish_rotation_increment`/`rotate_old_stress`
# settings under the hood. CSVDiff cases pull `decomposition_method` via cli_args.

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [all]
        strain = FINITE
        add_variables = true
        compatibility_mode = true
        volumetric_locking_correction = true
        decomposition_method = TaylorExpansion
        # In compatibility_mode, `mechanical_strain_xx` is redirected to read from
        # `rotated_mechanical_strain` (= what OLD `ComputeFiniteStrain` puts in
        # `_mechanical_strain`); `stress_xx` is redirected to read from `cauchy_stress`
        # (= what the wrapped plasticity material's FSR-rotated stress lives in).
        generate_output = 'stress_xx mechanical_strain_xx'
      []
    []
  []
[]

[Mesh]
  [msh]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
  []
[]

[Functions]
  [pullx]
    type = ParsedFunction
    expression = 't'
  []
  [pully]
    type = ParsedFunction
    expression = 't*0.25'
  []
  [pullz]
    type = ParsedFunction
    expression = 't*0.1'
  []
[]

[BCs]
  [leftx]
    type = DirichletBC
    preset = true
    boundary = left
    variable = disp_x
    value = 0.0
  []
  [lefty]
    type = DirichletBC
    preset = true
    boundary = bottom
    variable = disp_y
    value = 0.0
  []
  [leftz]
    type = DirichletBC
    preset = true
    boundary = back
    variable = disp_z
    value = 0.0
  []
  [pull_x]
    type = FunctionDirichletBC
    boundary = right
    variable = disp_x
    function = pullx
  []
  [pull_y]
    type = FunctionDirichletBC
    boundary = right
    variable = disp_y
    function = pully
  []
  [pull_z]
    type = FunctionDirichletBC
    boundary = right
    variable = disp_z
    function = pullz
  []
[]

[UserObjects]
  [./str]
    type = SolidMechanicsHardeningPowerRule
    value_0 = 100.0
    epsilon0 = 0.1
    exponent = 2.0
  [../]
  [./j2]
    type = SolidMechanicsPlasticJ2
    yield_strength = str
    yield_function_tolerance = 1E-3
    internal_constraint_tolerance = 1E-9
  [../]
[]

[Materials]
  [elastic_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 100000.0
    poissons_ratio = 0.3
  []
  [compute_stress_base]
    type = ComputeMultiPlasticityStress
    plastic_models = j2
    ep_plastic_tolerance = 1E-9
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  [./strain]
    type = ElementAverageValue
    variable = mechanical_strain_xx
  []
  [./stress]
    type = ElementAverageValue
    variable = stress_xx
  []
[]

[Executioner]
  type = Transient

  solve_type = 'newton'
  line_search = none

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  l_max_its = 2
  l_tol = 1e-14
  nl_max_its = 25
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10

  start_time = 0.0
  dt = 0.05
  dtmin = 0.05
  end_time = 0.5
[]

[Outputs]
  exodus = false
  [compare]
    type = CSV
  []
[]
