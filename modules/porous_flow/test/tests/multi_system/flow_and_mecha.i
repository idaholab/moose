PorousFlowDictatorName = 'dictator'

[GlobalParams]
  time_unit = days
  displacements = 'disp_x disp_y disp_z'
  use_displaced_mesh = false
[]

[Problem]
  kernel_coverage_check = false
  material_coverage_check = false
  nl_sys_names = 'porous_flow solid_mech'
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

[PorousFlowFullySaturated]
  coupling_type = HydroMechanical
  porepressure = porepressure
  biot_coefficient = 1
  fp = simple_fluid
  stabilization = FULL
  gravity = '0 0 0'
  add_darcy_aux = false
  dictator_name = ${PorousFlowDictatorName}
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
    family = LAGRANGE
    order = SECOND
    solver_sys = 'porous_flow'
  []
[]

[BCs]
  [fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'left right front'
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

  [porepressure_fix_left]
    type = DirichletBC
    variable = porepressure
    boundary = 'left top bottom'
    value = 2
  []
  [porepressure_fix_right]
    type = DirichletBC
    variable = porepressure
    boundary = 'right'
    value = 1
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 2E5
    density0 = 1000
    thermal_expansion = 1e-4
    viscosity = 9.0E-4
  []
[]

[Materials]
  [porosity_bulk]
    type = PorousFlowPorosityConst
    porosity = 0.3
    PorousFlowDictator = ${PorousFlowDictatorName}
  []
  [undrained_density_0]
    type = GenericConstantMaterial
    prop_names = density
    prop_values = 2500
  []
  [permeability_bulk]
    type = PorousFlowPermeabilityConst
    permeability = '1e-5 0 0 0 1e-5 0 0 0 1e-5'
    PorousFlowDictator = ${PorousFlowDictatorName}
  []

  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2e5
    poissons_ratio = 0.15
  []
  [finite_strain_stress]
    type = ComputeFiniteStrainElasticStress
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
  solve_type = NEWTON

  # best overall
  petsc_options = '-snes_converged_reason'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = ' lu       mumps'

  line_search = none

  nl_abs_tol = 5e-8
  nl_rel_tol = 1e-8

  l_max_its = 20
  nl_max_its = 12

  start_time = 0.0
  end_time = 1
  [TimeSteppers]
    [ConstantDT1]
      type = ConstantDT
      dt = 0.25
    []
  []
[]

[Postprocessors]
  [p000]
    type = PointValue
    variable = porepressure
    point = '0 0 0'
    execute_on = 'initial timestep_end'
  []
  [p550]
    type = PointValue
    variable = porepressure
    point = '5 5 0'
    execute_on = 'initial timestep_end'
  []
  [p-551]
    type = PointValue
    variable = porepressure
    point = '-5 -5 -1'
    execute_on = 'initial timestep_end'
  []
  [x000]
    type = PointValue
    variable = disp_x
    point = '0 0 0'
    execute_on = 'initial timestep_end'
  []
  [x550]
    type = PointValue
    variable = disp_x
    point = '5 5 0'
    execute_on = 'initial timestep_end'
  []
  [x-551]
    type = PointValue
    variable = disp_x
    point = '-5 -5 -1'
    execute_on = 'initial timestep_end'
  []
  [z000]
    type = PointValue
    variable = disp_z
    point = '0 0 0'
    execute_on = 'initial timestep_end'
  []
  [z550]
    type = PointValue
    variable = disp_z
    point = '5 5 0'
    execute_on = 'initial timestep_end'
  []
  [z-551]
    type = PointValue
    variable = disp_z
    point = '-5 -5 -1'
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  csv = true
[]
