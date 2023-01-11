[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
[]

[Variables/adjoint_u]
[]

[Kernels]
  [heat_conduction]
    type = Diffusion
    variable = adjoint_u
  []
[]

[DiracKernels]
  [pt]
    type = ReporterPointSource
    variable = adjoint_u
    x_coord_name = misfit/measurement_xcoord
    y_coord_name = misfit/measurement_ycoord
    z_coord_name = misfit/measurement_zcoord
    value_name = misfit/misfit_values
  []
[]

[BCs]
  [dirichlet]
    type = DirichletBC
    variable = adjoint_u
    boundary = 'bottom left'
    value = 0
  []
[]

[Reporters]
  [misfit]
    type = OptimizationData
  []
  [src_rep]
    type = ConstantReporter
    real_vector_names = 'vals'
    real_vector_values = '1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Functions]
  [src_func]
    type = ParameterMeshFunction
    exodus_mesh = parameter_mesh_in.e
    parameter_name = src_rep/vals
  []
[]

[VectorPostprocessors]
  [gradient_vpp]
    type = ElementOptimizationSourceFunctionInnerProduct
    variable = adjoint_u
    function = src_func
  []
[]

[Outputs]
  console = false
[]
