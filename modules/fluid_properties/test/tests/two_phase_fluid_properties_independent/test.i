# Tests the TwoPhaseFluidPropertiesIndependent class, which takes the names
# of 2 single-phase fluid properties independently. This test uses a dummy
# aux to make sure that the single-phase fluid properties can be recovered
# from the 2-phase fluid properties. A modification to this test checks that
# an error results if one tries to call a 2-phase fluid properties interface
# using this class, which is designed to ensure that the 2 phases are independent.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1

  # Required for NodalVariableValue on distributed mesh
  allow_renumbering = false
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [./p]
    initial_condition = 1e5
  [../]
  [./T]
    initial_condition = 300
  [../]
  [./rho_avg]
  [../]
[]

[FluidProperties]
  # rho1 = 1.149425287 kg/m^3
  [./fp1]
    type = IdealGasFluidProperties
    gamma = 1.4
    molar_mass = 0.02867055103448276
  [../]
  # rho2 = 0.6666666667 kg/m^3
  [./fp2]
    type = IdealGasFluidProperties
    gamma = 1.2
    molar_mass = 0.0166289196
  [../]
  [./fp_2phase]
    type = TwoPhaseFluidPropertiesIndependent
    fp_liquid = fp1
    fp_vapor = fp2
  [../]
[]

[AuxKernels]
  # correct value (0.5*(rho1 + rho2)) should be: 0.90804597685 kg/m^3
  [./rho_avg_aux]
    type = TwoPhaseAverageDensityAux
    variable = rho_avg
    p = p
    T = T
    fp_2phase = fp_2phase
    execute_on = 'initial'
  [../]
[]

[Postprocessors]
  [./rho_avg_value]
    type = NodalVariableValue
    variable = rho_avg
    nodeid = 0
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  execute_on = 'timestep_end'
  csv = true
[]
