# Apply a PorousFlowPointSourceFromPostprocessor that injects 1J/s into a 2D model, and PorousFlowOutflowBCs to the outer boundaries to show that the PorousFlowOutflowBCs allow heat-energy to exit freely at the appropriate rate
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  xmin = -1
  xmax = 1
  ny = 2
  ymin = -2
  ymax = 2
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[Variables]
  [pp]
  []
  [T]
    scaling = 1E-7
  []
[]

[PorousFlowFullySaturated]
  fp = simple_fluid
  coupling_type = thermohydro
  porepressure = pp
  temperature = T
[]

[DiracKernels]
  [injection]
    type = PorousFlowPointSourceFromPostprocessor
    mass_flux = 1
    point = '0 0 0'
    variable = T
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 1
  []
[]

[Materials]
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.12
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '0.4 0 0 0 0.4 0 0 0 0.4'
  []
  [matrix]
    type = PorousFlowMatrixInternalEnergy
    density = 0.15
    specific_heat_capacity = 1.5
  []
  [thermal_cond]
    type = PorousFlowThermalConductivityIdeal
    dry_thermal_conductivity = '0.3 0 0 0 0.3 0 0 0 0.3'
  []
[]

[BCs]
  [outflow]
    type = PorousFlowOutflowBC
    boundary = 'left right top bottom'
    flux_type = heat
    variable = T
    save_in = nodal_outflow
  []
[]

[AuxVariables]
  [nodal_outflow]
  []
[]

[Postprocessors]
  [outflow_J_per_s]
    type = NodalSum
    boundary = 'left right top bottom'
    variable = nodal_outflow
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
  dt = 2E6
  end_time = 2E7
  nl_abs_tol = 1E-14
#  nl_rel_tol = 1E-12
[]


[Outputs]
  csv = true
[]
