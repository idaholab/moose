[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 120
  ny = 1
  xmin = 0
  xmax = 6
  ymin = 0
  ymax = 0.05
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Functions]
  [dts]
    type = PiecewiseLinear
    y = '1E-2 1 10 500 5000 5000'
    x = '0 10 100 1000 10000 100000'
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = pressure
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.336
    alpha = 1.43e-4
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 2e7
    viscosity = 1.01e-3
    density0 = 1000
    thermal_expansion = 0
  []
[]

[Materials]
  [massfrac]
    type = PorousFlowMassFraction
  []
  [temperature]
    type = PorousFlowTemperature
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [ppss]
    type = PorousFlow1PhaseP
    porepressure = pressure
    capillary_pressure = pc
  []
  [relperm]
    type = PorousFlowRelativePermeabilityVG
    m = 0.336
    seff_turnover = 0.99
    phase = 0
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.33
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '0.295E-12 0 0  0 0.295E-12 0  0 0 0.295E-12'
  []
[]

[Variables]
  [pressure]
    initial_condition = -72620.4
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pressure
  []
  [flux0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pressure
    gravity = '-10 0 0'
  []
[]

[AuxVariables]
  [SWater]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [SWater]
    type = MaterialStdVectorAux
    property = PorousFlow_saturation_qp
    index = 0
    variable = SWater
  []
[]

[BCs]
  [base]
    type = PorousFlowSink
    boundary = right
    flux_function = -2.315E-3
    variable = pressure
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason -ksp_diagonal_scale -ksp_diagonal_scale_fix -ksp_gmres_modifiedgramschmidt -snes_linesearch_monitor'
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'gmres      asm      lu           NONZERO                   2               1E-10      1E-10      10'
  []
[]

[VectorPostprocessors]
  [swater]
    type = LineValueSampler
    warn_discontinuous_face_values = false
    variable = SWater
    start_point = '0 0 0'
    end_point = '6 0 0'
    sort_by = x
    num_points = 121
    execute_on = timestep_end
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  petsc_options = '-snes_converged_reason'
  end_time = 359424

  [TimeStepper]
    type = FunctionDT
    function = dts
  []
[]

[Outputs]
  file_base = rd01
  [exodus]
    type = Exodus
    execute_on = 'initial final'
  []
  [along_line]
    type = CSV
    execute_on = final
  []
[]
