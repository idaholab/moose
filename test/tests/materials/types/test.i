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
  [./stdvec0_qp0]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stdvec0_qp1]
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
  [./stdvecgrad00]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stdvecgrad01]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stdvecgrad02]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stdvecgrad10]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stdvecgrad11]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stdvecgrad12]
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
  [./stdvec0_qp0]
    type = MaterialStdVectorAux
    variable = stdvec0_qp0
    property = stdvec_prop_qp
    index = 0
    selected_qp = 0
    execute_on = timestep_end
  [../]
  [./stdvec0_qp1]
    type = MaterialStdVectorAux
    variable = stdvec0_qp1
    property = stdvec_prop_qp
    index = 0
    selected_qp = 1
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
  [./stdvecgrad00]
    type = MaterialStdVectorRealGradientAux
    variable = stdvecgrad00
    property = stdvec_grad_prop
  [../]
  [./stdvecgrad01]
    type = MaterialStdVectorRealGradientAux
    variable = stdvecgrad01
    property = stdvec_grad_prop
    component = 1
  [../]
  [./stdvecgrad02]
    type = MaterialStdVectorRealGradientAux
    variable = stdvecgrad02
    property = stdvec_grad_prop
    component = 2
  [../]
  [./stdvecgrad10]
    type = MaterialStdVectorRealGradientAux
    variable = stdvecgrad10
    index = 1
    property = stdvec_grad_prop
  [../]
  [./stdvecgrad11]
    type = MaterialStdVectorRealGradientAux
    variable = stdvecgrad11
    index = 1
    component = 1
    property = stdvec_grad_prop
  [../]
  [./stdvecgrad12]
    type = MaterialStdVectorRealGradientAux
    variable = stdvecgrad12
    index = 1
    component = 2
    property = stdvec_grad_prop
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
  solve_type = PJFNK
[]

[Outputs]
  file_base = test_out
  exodus = true
[]
