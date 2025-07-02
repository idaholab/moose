PorousFlowDictatorName = 'dictator'

[GlobalParams]
  time_unit = days
  displacements = 'disp_x disp_y disp_z'
  use_displaced_mesh = false
[]

[Problem]
  kernel_coverage_check = false
  material_coverage_check = false
  nl_sys_names = 'solid_mech porous_flow'
[]

[Mesh]
  [BaseMesh]
    type = GeneratedMeshGenerator
    subdomain_name = 'BaseMesh'
    elem_type = "TET10"
    dim = 3
    nx = 6
    ny = 6
    nz = 2
    xmin = -10
    xmax = +10
    ymin = -10
    ymax = +10
    zmin = -2
    zmax = +2
  []
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [all]
        strain = SMALL
        incremental = true
      []
    []
  []
[]

[AuxVariables]
  [stress_xx]
    order = CONSTANT
    family = MONOMIAL
    block = 'BaseMesh'
  []
  [stress_yy]
    order = CONSTANT
    family = MONOMIAL
    block = 'BaseMesh'
  []
  [stress_zz]
    order = CONSTANT
    family = MONOMIAL
    block = 'BaseMesh'
  []
[]

[PorousFlowFullySaturated]
  coupling_type = HydroMechanical
  porepressure = porepressure
  biot_coefficient = 1
  fp = simple_fluid
  stabilization = FULL
  gravity = '0 0 0'
  add_darcy_aux = false
  dictator_name = ${PorousFlowDictatorName}
  block = 'BaseMesh'
[]

[Variables]
  [disp_x]
    solver_sys = 'solid_mech'
  []
  [disp_y]
    solver_sys = 'solid_mech'
  []
  [disp_z]
    solver_sys = 'solid_mech'
  []
  [porepressure]
    order = SECOND
    family = LAGRANGE
    scaling = 1e-5
    block = 'BaseMesh'
    solver_sys = 'porous_flow'
  []
[]

[ICs]
  [porepressure]
    type = FunctionIC
    variable = porepressure
    function = '2'
    block = 'BaseMesh'
  []
[]

[BCs]

  [fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'left right'
    value = 0.0
  []

  [fix_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom top'
    value = 0.0
  []

  [fix_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'back'
    value = 0.0
  []

  [porepressure_fix]
    type = DirichletBC
    variable = porepressure
    boundary = 'left right bottom top'
    value = 2.0
  []

  [porepressure_Boundary]
    type = FunctionDirichletBC
    variable = porepressure
    boundary = 'left'
    function = porepressure_at_left
  []
[]

[Functions]
  [porepressure_at_left]
    type = ParsedFunction
    expression = '2 + max(0, min(1, t-0.25))'
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 2E3
    density0 = 1000
    thermal_expansion = 0
    viscosity = 9.0E-10
  []
[]

[Materials]

  [porosity_bulk]
    type = PorousFlowPorosityConst
    porosity = 0.15
    PorousFlowDictator = ${PorousFlowDictatorName}
  []

  [undrained_density_0]
    type = GenericConstantMaterial
    prop_names = density
    prop_values = 2500
  []

  [permeability_bulk]
    type = PorousFlowPermeabilityConst
    block = 'BaseMesh'
    permeability = '1e-5 0 0 0 1e-5 0 0 0 1e-5'
    PorousFlowDictator = ${PorousFlowDictatorName}
  []

  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = 'BaseMesh'
    youngs_modulus = 2500
    poissons_ratio = 0.15
  []

  [stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = ''
    perform_finite_strain_rotations = false
    tangent_operator = 'nonlinear'
  []
[]

[Preconditioning]
  [SMP_porous]
    type = SMP
    full = true
    nl_sys = 'porous_flow'
  []
  [SMP_mecha]
    type = SMP
    full = true
    nl_sys = 'solid_mech'
  []
[]

[Executioner]
  type = Transient
  solve_type = PJFNK

  petsc_options = '-snes_converged_reason'

  # best overall
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = ' lu       mumps'

  line_search = none

  nl_abs_tol = 1e-4
  nl_rel_tol = 1e-6

  l_max_its = 20
  nl_max_its = 8

  start_time = 0.0
  end_time = 0.5
  [TimeSteppers]
    [ConstantDT1]
      type = ConstantDT
      dt = 0.25
    []
  []

  [Quadrature]
    type = SIMPSON
    order = SECOND
  []
[]

[Outputs]
  perf_graph = true
  exodus = true
[]
