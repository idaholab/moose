# Apply a PorousFlowPointSourceFromPostprocessor that injects 1kg/s into a 2D model, and PorousFlowOutflowBCs to the outer boundaries to show that the PorousFlowOutflowBCs allow fluid to exit freely at the appropriate rate
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
[]

[PorousFlowFullySaturated]
  fp = simple_fluid
  porepressure = pp
[]

[DiracKernels]
  [injection]
    type = PorousFlowPointSourceFromPostprocessor
    mass_flux = 1
    point = '0 0 0'
    variable = pp
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
[]

[BCs]
  [outflow]
    type = PorousFlowOutflowBC
    boundary = 'left right top bottom'
    variable = pp
    save_in = nodal_outflow
  []
[]

[AuxVariables]
  [nodal_outflow]
  []
[]

[Postprocessors]
  [outflow_kg_per_s]
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
  dt = 3E-4
  end_time = 30E-4
  nl_abs_tol = 1E-9
  nl_rel_tol = 1E-9
[]


[Outputs]
  csv = true
[]
