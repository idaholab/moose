[Mesh]
  [ebsd_mesh]
    type = EBSDMeshGenerator
    filename = IN100_001_28x28_Clean_Marmot.txt
  []
[]

[UserObjects]
  [ebsd]
    type = EBSDReader
  []
[]

[AuxVariables]
  [phi1]
    family = MONOMIAL
    order = CONSTANT
  []
  [phi]
    family = MONOMIAL
    order = CONSTANT
  []
  [phi2]
    family = MONOMIAL
    order = CONSTANT
  []

  [phase]
    order = CONSTANT
    family = MONOMIAL
  []
  [symmetry]
    order = CONSTANT
    family = MONOMIAL
  []

  [rgb]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [phi1_aux]
    type = EBSDReaderPointDataAux
    variable = phi1
    ebsd_reader = ebsd
    data_name = phi1
    execute_on = initial
  []
  [phi_aux]
    type = EBSDReaderPointDataAux
    variable = phi
    ebsd_reader = ebsd
    data_name = phi
    execute_on = initial
  []
  [phi2_aux]
    type = EBSDReaderPointDataAux
    variable = phi2
    ebsd_reader = ebsd
    data_name = phi2
    execute_on = initial
  []

  [phase_aux]
    type = EBSDReaderPointDataAux
    variable = phase
    ebsd_reader = ebsd
    data_name = phase
    execute_on = initial
  []

  [symmetry_aux]
    type = EBSDReaderPointDataAux
    variable = symmetry
    ebsd_reader = ebsd
    data_name = symmetry
    execute_on = initial
  []

  [rgb]
    type = EulerAngleVariables2RGBAux
    variable = rgb
    phi1 = phi1
    phi = phi
    phi2 = phi2
    phase = phase
    symmetry = symmetry
    execute_on = initial
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Outputs]
  exodus = true
  show = 'rgb'
[]
