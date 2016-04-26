# Buckley-Leverett 1-phase.
# The front starts at (around) x=5, and at t=50 it should
# have moved to x=9.6.  The version below has a nonzero
# suction function, and at t=50, the front sits between
# (about) x=9.7 and x=10.4.  Changing the van-Genuchten
# al parameter to 1E-3 sharpens the front so it sits between
# x=9.6 and x=9.9, but of course the simulation takes longer
# with al=1E-2 and nx=600, the front sits between x=9.6 and x=9.8,
# but takes about 100 times longer to run.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 150
  xmin = 0
  xmax = 15
[]

[GlobalParams]
  PorousFlowDictator_UO = dictator
[]

[Variables]
  [./pp]
    [./InitialCondition]
      type = FunctionIC
      function = 'max((1000000-x/5*1000000)-20000,-20000)'
    [../]
  [../]
[]

[Kernels]
  [./mass0]
    type = PorousFlowMassTimeDerivative
    component_index = 0
    variable = pp
  [../]
  [./flux0]
    type = PorousFlowAdvectiveFlux
    component_index = 0
    variable = pp
    gravity = '0 0 0'
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = pp
    boundary = left
    value = 980000
  [../]
[]

[AuxVariables]
  [./sat]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[AuxKernels]
  [./sat]
    type = MaterialStdVectorAux
    variable = sat
    execute_on = timestep_end
    index = 0
    property = PorousFlow_saturation_qp
  [../]
[]


[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  [../]
[]

[Materials]
  [./ppss]
    type = PorousFlowMaterial1PhaseP_VG
    porepressure = pp
    al = 1E-4
    m = 0.8
  [../]
  [./massfrac]
    type = PorousFlowMaterialMassFractionBuilder
  [../]
  [./dens0]
    type = PorousFlowMaterialDensityConstBulk
    density_P0 = 1000
    bulk_modulus = 2.0E6
    phase = 0
  [../]
  [./dens_all]
    type = PorousFlowMaterialJoinerOld
    material_property = PorousFlow_fluid_phase_density
  [../]
  [./dens_qp_all]
    type = PorousFlowMaterialJoiner
    material_property = PorousFlow_fluid_phase_density_qp
    use_qps = true
  [../]
  [./visc0]
    type = PorousFlowMaterialViscosityConst
    viscosity = 1E-3
    phase = 0
  [../]
  [./visc_all]
    type = PorousFlowMaterialJoiner
    material_property = PorousFlow_viscosity
  [../]
  [./permeability]
    type = PorousFlowMaterialPermeabilityConst
    permeability = '1E-10 0 0  0 1E-10 0  0 0 1E-10'
  [../]
  [./relperm]
    type = PorousFlowMaterialRelativePermeabilityCorey
    n_j = 2
    phase = 0
  [../]
  [./relperm_all]
    type = PorousFlowMaterialJoiner
    material_property = PorousFlow_relative_permeability
  [../]
  [./porosity]
    type = PorousFlowMaterialPorosityConst
    porosity = 0.15
  [../]
[]


[Preconditioning]
  active = andy
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'gmres bjacobi 1E-10 1E-10 20'
  [../]
[]

[Executioner]
  type = Transient
  end_time = 50
  dt = 2
[]

[Outputs]
  file_base = bl01
  execute_on = 'final'
  exodus = true
[]
