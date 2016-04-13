[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
[]

[Variables]
  [./pp]
  [../]
  [./sat]
    initial_condition = 0.5
  [../]
[]

[AuxVariables]
  [./gasph_gascomp]
    # the mass fraction of the gas component in the gas phase - in this example it remains fixed at 1
    initial_condition = 1
  [../]
  [./waterph_gascomp]
    # the mass fraction of the gas component in the water phase - in this example it remains fixed at 0
    initial_condition = 0
  [../]
[]

[Kernels]
  [./mass0]
    type = PorFlowComponentMassTimeDerivative
    component_index = 0
    PorFlowVarNames_UO = porflow_vars
    variable = pp
  [../]
  [./mass1]
    type = PorFlowComponentMassTimeDerivative
    component_index = 1
    PorFlowVarNames_UO = porflow_vars
    variable = sat
  [../]
[]

[UserObjects]
  [./porflow_vars]
    type = PorFlowVarNames
    porflow_vars = 'pp sat'
  [../]
[]

[Materials]
  [./ppss]
    type = PorFlowMaterial2PhasePS
    phase0_porepressure = pp
    phase1_saturation = sat
    PorFlowVarNames_UO = porflow_vars
  [../]
  [./massfrac]
    type = PorFlowMaterialMassFractionBuilder
    num_phases = 2
    num_components = 2
    mass_fraction_vars = 'gasph_gascomp waterph_gascomp'
    PorFlowVarNames_UO = porflow_vars
  [../]
  [./dens0]
    type = PorFlowMaterialDensityConstBulk
    density0 = 1
    bulk_modulus = 1.5
    phase = 0
    PorFlowVarNames_UO = porflow_vars
  [../]
  [./dens1]
    type = PorFlowMaterialDensityConstBulk
    density0 = 0.5
    bulk_modulus = 0.5
    phase = 1
    PorFlowVarNames_UO = porflow_vars
  [../]
  [./dens_all]
    type = PorFlowMaterialDensityBuilder
    num_phases = 2
    PorFlowVarNames_UO = porflow_vars
  [../]
  [./porosity]
    type = PorFlowMaterialPorosityConst
    porosity = 0.1
    PorFlowVarNames_UO = porflow_vars
  [../]
[]

[Preconditioning]
  active = andy
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000'
  [../]
  [./check]
    type = SMP
    full = true
    petsc_options = '-snes_test_display'
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -snes_type'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000 test'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 2
[]

[Outputs]
  exodus = true
[]
