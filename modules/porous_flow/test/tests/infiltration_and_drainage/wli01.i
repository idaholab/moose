[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1000
  ny = 1
  xmin = -10000
  xmax = 0
  ymin = 0
  ymax = 0.05
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = pressure
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureBW
    Sn = 0.0
    Ss = 1.0
    C = 1.5
    las = 2
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 2e9
    viscosity = 4
    density0 = 10
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
    type = PorousFlowRelativePermeabilityBW
    Sn = 0.0
    Ss = 1.0
    Kn = 0
    Ks = 1
    C = 1.5
    phase = 0
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.25
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1 0 0  0 1 0  0 0 1'
  []
[]

[Variables]
  [pressure]
    initial_condition = -1E-4
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
    gravity = '-0.1 0 0'
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
    type = DirichletBC
    boundary = 'left'
    value = -1E-4
    variable = pressure
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason -ksp_diagonal_scale -ksp_diagonal_scale_fix -ksp_gmres_modifiedgramschmidt -snes_linesearch_monitor'
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'gmres      asm      lu           NONZERO                   2               1E-10      1E-10      10000'
  []
[]

[VectorPostprocessors]
  [swater]
    type = LineValueSampler
    warn_discontinuous_face_values = false
    variable = SWater
    start_point = '-5000 0 0'
    end_point = '0 0 0'
    sort_by = x
    num_points = 71
    execute_on = timestep_end
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  petsc_options = '-snes_converged_reason'
  end_time = 1000
  dt = 1
[]

[Outputs]
  file_base = wli01
  sync_times = '100 500 1000'
  [exodus]
    type = Exodus
    sync_only = true
  []
  [along_line]
    type = CSV
    sync_only = true
  []
[]
