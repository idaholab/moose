# Example problem: Elder, Transient convection in a porous mediu, 1967

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 64
    ny = 32
    xmin = 0
    xmax = 300
    ymax = 0
    ymin = -150
  []
  [heater]
    type = ParsedGenerateSideset
    input = gen
    combinatorial_geometry = 'x <= 150 & y = -150'
    new_sideset_name = heater
  []
  uniform_refine = 1
[]

[Variables]
  [porepressure]
  []
  [T]
    initial_condition = 285
  []
[]

[AuxVariables]
  [density]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [density]
    type = PorousFlowPropertyAux
    variable = density
    property = density
    execute_on = TIMESTEP_END
  []
[]

[ICs]
  [hydrostatic]
    type = FunctionIC
    variable = porepressure
    function = '1e5 - 9.81 * 1000 * y'
  []
[]

[GlobalParams]
  PorousFlowDictator = 'dictator'
  gravity = '0 -9.81 0'
[]

[FluidProperties]
  [water]
    type = Water97FluidProperties
  []
[]

[PorousFlowFullySaturated]
  coupling_type = ThermoHydro
  porepressure = porepressure
  temperature = T
  fp = water
[]

[Materials]
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1.21E-10 0 0   0 1.21E-10 0   0 0 1.21E-10'
  []
  [Matrix_internal_energy]
    type = PorousFlowMatrixInternalEnergy
    density = 2500
    specific_heat_capacity = 0
  []
  [thermal_conductivity]
    type = PorousFlowThermalConductivityIdeal
    dry_thermal_conductivity = '1.5 0 0  0 1.5 0  0 0 0'
  []
[]

[BCs]
  [t_bot]
    type = DirichletBC
    variable = T
    value = 293
    boundary = 'heater'
  []
  [t_top]
    type = DirichletBC
    variable = T
    value = 285
    boundary = 'top'
  []
  [p_top]
    type = DirichletBC
    variable = porepressure
    value = 1e5
    boundary = top
  []
[]

[Preconditioning]
  [basic]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap'
    petsc_options_value = ' asm      lu           NONZERO                   2'
  []
[]

[Executioner]
  type = Transient
  end_time = 63072000
  dtmax = 1e6
  nl_rel_tol = 1e-6
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1000
  []
  [Adaptivity]
    interval = 1
    refine_fraction = 0.2
    coarsen_fraction = 0.3
    max_h_level = 4
  []
[]

[Outputs]
  exodus = true
[]

# If you uncomment this it will print out all the kernels and materials that the PorousFlowFullySaturated action generates
#[Problem]
#  type = DumpObjectsProblem
#  dump_path = PorousFlowFullySaturated
#[]
