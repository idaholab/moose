[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 11
  ny = 11
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 1.0
[]

[Variables]
  [./forced]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = forced
  [../]

  [./forcing]
    type = BodyForce
    variable = forced
    function = 'x*x+y*y' # Any object expecting a function name can also receive a ParsedFunction string
  [../]
[]

[BCs]
  [./all]
    type = DirichletBC
    variable = forced
    boundary = 'bottom right top left'
    value = 0
  [../]
[]

[UserObjects]
  [./fine_solution]
    # Read in the fine grid solution
    type = SolutionUserObject
    system_variables = forced
    mesh = ex14_compare_solutions_1_out_0000_mesh.xda
    es = ex14_compare_solutions_1_out_0000.xda
  [../]
[]

[Functions]
  [./fine_function]
    # Create a Function out of the fine grid solution
    # Note: This references the SolutionUserObject above
    type = SolutionFunction
    solution = fine_solution
  [../]
[]

[Executioner]
  type = Steady

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  [./Quadrature]
    # The integration of the error happens on the coarse mesh
    # To reduce integration error of the finer solution we can
    # raise the integration order.
    # Note: This will slow down the calculation a bit
    order = SIXTH
  [../]
[]

[Postprocessors]
  [./error]
    # Compute the error between the computed solution and the fine-grid solution
    type = ElementL2Error
    variable = forced
    function = fine_function
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
