# PorousFlowBasicTHM action with coupling_type = HydroMechanical

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 3
    xmax = 10
    ymax = 3
  []
  [aquifer]
    input = gen
    type = SubdomainBoundingBoxGenerator
    block_id = 1
    bottom_left = '0 1 0'
    top_right = '10 2 0'
  []
  [injection_area]
    type = SideSetsAroundSubdomainGenerator
    block = 1
    new_boundary = 'injection_area'
    normal = '-1 0 0'
    input = 'aquifer'
  []
  [outflow_area]
    type = SideSetsAroundSubdomainGenerator
    block = 1
    new_boundary = 'outflow_area'
    normal = '1 0 0'
    input = 'injection_area'
  []
  [rename]
    type = RenameBlockGenerator
    old_block = '0 1'
    new_block = 'caprock aquifer'
    input = 'outflow_area'
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
  displacements = 'disp_x disp_y'
  biot_coefficient = 1.0
[]

[Variables]
  [porepressure]
    initial_condition = 1e6
  []
  [disp_x]
    scaling = 1e-10
  []
  [disp_y]
    scaling = 1e-10
  []
[]

[AuxVariables]
  [temperature]
    initial_condition = 293
  []
[]

[PorousFlowBasicTHM]
  porepressure = porepressure
  temperature = temperature
  coupling_type = HydroMechanical
  gravity = '0 0 0'
  fp = simple_fluid
  use_displaced_mesh = false
  add_stress_aux = false
[]

[BCs]
  [constant_injection_porepressure]
    type = DirichletBC
    variable = porepressure
    value = 1.5e6
    boundary = injection_area
  []
  [constant_outflow_porepressure]
    type = PorousFlowPiecewiseLinearSink
    variable = porepressure
    boundary = outflow_area
    pt_vals = '0 1e9'
    multipliers = '0 1e9'
    flux_function = 1e-6
    PT_shift = 1e6
  []
  [top_bottom]
    type = DirichletBC
    variable = disp_y
    value = 0
    boundary = 'top bottom'
  []
  [right]
    type = DirichletBC
    variable = disp_x
    value = 0
    boundary = right
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
  []
[]

[Materials]
  [porosity]
    type = PorousFlowPorosity
    porosity_zero = 0.1
  []
  [biot_modulus]
    type = PorousFlowConstantBiotModulus
    solid_bulk_compliance = 2e-7
    fluid_bulk_modulus = 1e7
  []
  [permeability_aquifer]
    type = PorousFlowPermeabilityConst
    block = aquifer
    permeability = '1e-13 0 0   0 1e-13 0   0 0 1e-13'
  []
  [permeability_caprock]
    type = PorousFlowPermeabilityConst
    block = caprock
    permeability = '1e-15 0 0   0 1e-15 0   0 0 1e-15'
  []
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 5e9
    poissons_ratio = 0.0
  []
  [strain]
    type = ComputeSmallStrain
  []
  [stress]
    type = ComputeLinearElasticStress
  []
[]

[Preconditioning]
  [basic]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 1e4
  dt = 1e3
  nl_abs_tol = 1e-14
  nl_rel_tol = 1e-14
[]

[Outputs]
  exodus = true
[]
