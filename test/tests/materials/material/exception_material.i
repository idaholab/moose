[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./mat]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./time_derivative]
    type = TimeDerivative
    variable = u
  [../]
  [./diff]
    type = MatDiffusionTest
    variable = u
    prop_name = matp
  [../]
  [./f]
    type = BodyForce
    variable = u
    function = '20'
  [../]
[]

[AuxKernels]
  [./mat]
    # Sequence of events:
    # 1.) MaterialRealAux is re-evaluated every linear iteration
    # 2.) MaterialRealAux calls ExceptionMaterial::computeQpProperties()
    # 3.) ExceptionMaterial throws a MooseException.
    # 4.) The MooseException is caught and handled by MOOSE.
    # 5.) The next solve is automatically failed.
    # 6.) Time timestep is cut and we try again.
    #
    # The idea is to test that MOOSE can recover when exceptions are
    # thrown during AuxKernel evaluation, and not just nonlinear
    # residual/jacobian evaluation.
    type = MaterialRealAux
    variable = mat
    property = matp
  [../]
[]

[BCs]
  [./all]
    type = DirichletBC
    variable = u
    boundary = 'left top bottom right'
    value = 0
  [../]
[]

[Materials]
  [./mat]
    type = ExceptionMaterial
    block = 0
    rank = 0
    coupled_var = u
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.1
  end_time = .5
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
