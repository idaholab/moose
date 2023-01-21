# Groundwater extraction example.
# System consists of two confined aquifers separated by an aquitard
# There is a hydraulic gradient in the upper aquifer
# A well extracts water from the lower aquifer, and the impact on the upper aquifer is observed
# In the center of the model, the roof of the upper aquifer sits 70m below the local water table

[Mesh]
  [basic_mesh]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = -50
    xmax = 50
    nx = 20
    ymin = -25
    ymax = 25
    ny = 10
    zmin = -100
    zmax = -70
    nz = 3
  []
  [lower_aquifer]
    type = SubdomainBoundingBoxGenerator
    input = basic_mesh
    block_id = 1
    block_name = lower_aquifer
    bottom_left = '-1000 -500 -100'
    top_right = '1000 500 -90'
  []
  [aquitard]
    type = SubdomainBoundingBoxGenerator
    input = lower_aquifer
    block_id = 2
    block_name = aquitard
    bottom_left = '-1000 -500 -90'
    top_right = '1000 500 -80'
  []
  [upper_aquifer]
    type = SubdomainBoundingBoxGenerator
    input = aquitard
    block_id = 3
    block_name = upper_aquifer
    bottom_left = '-1000 -500 -80'
    top_right = '1000 500 -70'
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [pp]
  []
[]

[ICs]
  [pp]
    type = FunctionIC
    variable = pp
    function = insitu_pp
  []
[]

[BCs]
  [pp]
    type = FunctionDirichletBC
    variable = pp
    function = insitu_pp
    boundary = 'left right top bottom front back'
  []
[]

[Functions]
  [upper_aquifer_head]
    type = ParsedFunction
    expression = '10 + x / 200'
  []
  [lower_aquifer_head]
    type = ParsedFunction
    expression = '20'
  []
  [insitu_head]
    type = ParsedFunction
    symbol_values = 'lower_aquifer_head upper_aquifer_head'
    symbol_names = 'low up'
    expression = 'if(z <= -90, low, if(z >= -80, up, (up * (z + 90) - low * (z + 80)) / (10.0)))'
  []
  [insitu_pp]
    type = ParsedFunction
    symbol_values = 'insitu_head'
    symbol_names = 'h'
    expression = '(h - z) * 1E4'
  []
  [l_rate]
    type = ParsedFunction
    symbol_values = 'm3_produced dt'
    symbol_names = 'm3_produced dt'
    expression = '1000 * m3_produced / dt'
  []
[]

[AuxVariables]
  [insitu_head]
  []
  [head_change]
  []
[]

[AuxKernels]
  [insitu_head]
    type = FunctionAux
    variable = insitu_head
    function = insitu_head
  []
  [head_change]
    type = ParsedAux
    coupled_variables = 'pp insitu_head'
    use_xyzt = true
    expression = 'pp / 1E4 + z - insitu_head'
    variable = head_change
  []
[]

[Postprocessors]
  [m3_produced]
    type = PorousFlowPlotQuantity
    uo = volume_extracted
    outputs = 'none'
  []
  [dt]
    type = TimestepSize
    outputs = 'none'
  []
  [l_per_s]
    type = FunctionValuePostprocessor
    function = l_rate
  []
[]

[VectorPostprocessors]
  [drawdown]
    type = LineValueSampler
    variable = head_change
    start_point = '-50 0 -75'
    end_point = '50 0 -75'
    num_points = 101
    sort_by = x
  []
[]

[PorousFlowBasicTHM]
  fp = simple_fluid
  gravity = '0 0 -10'
  porepressure = pp
  multiply_by_density = false
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    # the following mean that density = 1000 * exp(P / 1E15) ~ 1000
    thermal_expansion = 0
    bulk_modulus = 1E15
  []
[]

[Materials]
  [porosity_aquifers]
    type = PorousFlowPorosityConst
    porosity = 0.05
    block = 'upper_aquifer lower_aquifer'
  []
  [porosity_aquitard]
    type = PorousFlowPorosityConst
    porosity = 0.2
    block = aquitard
  []
  [biot_mod]
    type = PorousFlowConstantBiotModulus
    fluid_bulk_modulus = 2E9
    biot_coefficient = 1.0
  []
  [permeability_aquifers]
    type = PorousFlowPermeabilityConst
    permeability = '1E-12 0 0 0 1E-12 0 0 0 1E-12'
    block = 'upper_aquifer lower_aquifer'
  []
  [permeability_aquitard]
    type = PorousFlowPermeabilityConst
    permeability = '1E-16 0 0 0 1E-16 0 0 0 1E-17'
    block = aquitard
  []
[]

[DiracKernels]
  [sink]
    type = PorousFlowPolyLineSink
    SumQuantityUO = volume_extracted
    point_file = ex01.bh_lower
    line_length = 10
    variable = pp
    # following produces a flux of 0 m^3(water)/m(borehole length)/s if porepressure = 0, and a flux of 1 m^3/m/s if porepressure = 1E9
    p_or_t_vals = '0 1E9'
    fluxes = '0 1'
  []
[]

[UserObjects]
  [volume_extracted]
    type = PorousFlowSumQuantity
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  [TimeStepper]
    type = SolutionTimeAdaptiveDT
    dt = 1.1E5
  []
  end_time = 3.456E5 # 4 days
  nl_abs_tol = 1E-13
[]

[Outputs]
  [csv]
    type = CSV
    file_base = ex01_lower_extraction
    execute_on = final
  []
[]
