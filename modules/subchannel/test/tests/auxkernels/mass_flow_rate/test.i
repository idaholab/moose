[Mesh]
    type = GeneratedMesh
    dim = 1
  []

  [AuxVariables]
    [S]
    []

    [mdot]
    []
  []

  [ICs]
    [S_IC]
      type = ConstantIC
      variable = S
      value = 0.5
    []
  []

  [AuxKernels]
    [mdot_ak]
      type = SCMMassFlowRateAux
      variable = mdot
      area = S
      mass_flux = 4722
      execute_on = 'initial'
    []
  []

  [Problem]
    solve = false
  []

  [Executioner]
    type = Steady
  []

  [Outputs]
    exodus = true
  []

