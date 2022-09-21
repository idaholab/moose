# The sample is a single unit element in RZ coordinates
# A constant velocity is applied to the outer boundary is free to move as a source injects heat and fluid into the system
# There is no fluid flow or heat flow.
# Heat energy conservation is checked.
# Mass conservation is checked
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
  xmin = 1
  xmax = 2
  ymin = -0.5
  ymax = 0.5
[]

[Problem]
  coord_type = RZ
[]

[GlobalParams]
  displacements = 'disp_r disp_z'
  PorousFlowDictator = dictator
  block = 0
  biot_coefficient = 0.3
[]

[Variables]
  [disp_r]
  []
  [disp_z]
  []
  [pp]
    initial_condition = 0.1
  []
  [temp]
    initial_condition = 10
  []
[]

[BCs]
  [plane_strain]
    type = DirichletBC
    variable = disp_z
    value = 0
    boundary = 'bottom top'
  []
  [rmin_fixed]
    type = DirichletBC
    variable = disp_r
    value = 0
    boundary = left
  []
  [contract]
    type = FunctionDirichletBC
    variable = disp_r
    function = -0.01*t
    boundary = right
  []
[]

[PorousFlowFullySaturated]
  coupling_type = ThermoHydroMechanical
  porepressure = pp
  temperature = temp
  fp = simple_fluid
[]

[DiracKernels]
  [heat_source]
    type = PorousFlowPointSourceFromPostprocessor
    point = '1.5 0 0'
    variable = temp
    mass_flux = 10
  []
  [fluid_source]
    type = PorousFlowPointSourceFromPostprocessor
    point = '1.5 0 0'
    variable = pp
    mass_flux = 1
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 0.5
    density0 = 1
    viscosity = 1
    thermal_expansion = 0
    cv = 1.3
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '1 1.5'
    # bulk modulus is lambda + 2*mu/3 = 1 + 2*1.5/3 = 2
    fill_method = symmetric_isotropic
  []
  [strain]
    type = ComputeAxisymmetricRZSmallStrain
  []
  [stress]
    type = ComputeLinearElasticStress
  []
  [porosity]
    type = PorousFlowPorosity
    porosity_zero = 0.1
  []
  [rock_heat]
    type = PorousFlowMatrixInternalEnergy
    specific_heat_capacity = 2.2
    density = 0.5
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '0.5 0 0   0 0.5 0   0 0 0.5'
  []
  [thermal_cond]
    type = PorousFlowThermalConductivityIdeal
    dry_thermal_conductivity = '1 0 0  0 1 0  0 0 1'
  []
[]

[Postprocessors]
  [p0]
    type = PointValue
    outputs = 'console csv'
    execute_on = 'initial timestep_end'
    point = '1 0 0'
    variable = pp
  []
  [t0]
    type = PointValue
    outputs = 'console csv'
    execute_on = 'initial timestep_end'
    point = '1 0 0'
    variable = temp
  []
  [rdisp]
    type = PointValue
    outputs = 'csv console'
    point = '2 0 0'
    use_displaced_mesh = false
    variable = disp_r
  []
  [fluid_mass]
    type = PorousFlowFluidMass
    fluid_component = 0
    execute_on = 'initial timestep_end'
    outputs = 'console csv'
  []
  [total_heat]
    type = PorousFlowHeatEnergy
    phase = 0
    execute_on = 'initial timestep_end'
    outputs = 'console csv'
  []
  [rock_heat]
    type = PorousFlowHeatEnergy
    execute_on = 'initial timestep_end'
    outputs = 'console csv'
  []
  [fluid_heat]
    type = PorousFlowHeatEnergy
    include_porous_skeleton = false
    phase = 0
    execute_on = 'initial timestep_end'
    outputs = 'console csv'
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 2
  end_time = 10
[]

[Outputs]
  execute_on = 'initial timestep_end'
  [csv]
    type = CSV
  []
[]
