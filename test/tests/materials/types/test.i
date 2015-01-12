[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 2
  ny = 2
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./real]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./stdvec0]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stdvec1]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./realvec0]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./realvec1]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./realvec2]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./densemat00]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./densemat01]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./tensor00]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./tensor11]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./tensor22]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  [./real0]
    type = MaterialRealAux
    variable = real
    property = real_prop
    execute_on = timestep_end
  [../]

  [./stdvec0]
    type = MaterialStdVectorAux
    variable = stdvec0
    property = stdvec_prop
    index = 0
    execute_on = timestep_end
  [../]
  [./stdvec1]
    type = MaterialStdVectorAux
    variable = stdvec1
    property = stdvec_prop
    index = 1
    execute_on = timestep_end
  [../]

  [./densemat00]
    type = MaterialRealDenseMatrixAux
    variable = densemat00
    property = matrix_prop
    row = 0
    column = 0
    execute_on = timestep_end
  [../]
  [./densemat01]
    type = MaterialRealDenseMatrixAux
    variable = densemat01
    property = matrix_prop
    row = 0
    column = 1
    execute_on = timestep_end
  [../]

  [./realvec0]
    type = MaterialRealVectorValueAux
    variable = realvec0
    property = realvec_prop
    component = 0
    execute_on = timestep_end
  [../]
  [./realvec1]
    type = MaterialRealVectorValueAux
    variable = realvec1
    property = realvec_prop
    component = 1
    execute_on = timestep_end
  [../]
  [./realvec2]
    type = MaterialRealVectorValueAux
    variable = realvec2
    property = realvec_prop
    component = 2
    execute_on = timestep_end
  [../]

  [./realtensor00]
    type = MaterialRealTensorValueAux
    variable = tensor00
    property = tensor_prop
    row = 0
    column = 0
    execute_on = timestep_end
  [../]
  [./realtensor11]
    type = MaterialRealTensorValueAux
    variable = tensor11
    property = tensor_prop
    row = 1
    column = 1
    execute_on = timestep_end
  [../]
  [./realtensor22]
    type = MaterialRealTensorValueAux
    variable = tensor22
    property = tensor_prop
    row = 2
    column = 2
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]
[]

[Materials]
  [./mat]
    type = TypesMaterial
    block = 0
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  output_on = 'initial timestep_end'
  [./out]
    type = Exodus
  [../]
  [./console]
    type = Console
    perf_log = true
    output_on = 'timestep_end failed nonlinear'
  [../]
[]
