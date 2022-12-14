# DO NOT CHANGE THIS TEST
# this test is documented as an example in forceInv_pointLoads.md
# if this test is changed, the figures will need to be updated.
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmax = 1
  ymax = 1.4
[]

[Variables]
  [adjoint]
  []
[]

[Problem]
  extra_tag_vectors = 'ref'
[]
[AuxVariables]
  [residual_src]
  []
[]
[AuxKernels]
  [residual_src]
    type = TagVectorAux
    vector_tag = 'ref'
    v = 'adjoint'
    variable = 'residual_src'
  []
[]

[Variables]
  [adjoint]
  []
[]

[Kernels]
  [heat_conduction]
    type = MatDiffusion
    variable = adjoint
    diffusivity = thermal_conductivity
  []
[]

#-----every adjoint problem should have these two
[DiracKernels]
  [pt]
    type = ReporterPointSource
    variable = adjoint
    x_coord_name = misfit/measurement_xcoord
    y_coord_name = misfit/measurement_ycoord
    z_coord_name = misfit/measurement_zcoord
    value_name = misfit/misfit_values
    extra_vector_tags = 'ref'
  []
[]

[Reporters]
  [misfit]
    type = OptimizationData
  []
[]
#---------------------------------------------------

[BCs]
  [left]
    type = DirichletBC
    variable = adjoint
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = adjoint
    boundary = right
    value = 0
  []
  [bottom]
    type = DirichletBC
    variable = adjoint
    boundary = bottom
    value = 0
  []
  [top]
    type = DirichletBC
    variable = adjoint
    boundary = top
    value = 0
  []
[]

[Materials]
  [steel]
    type = GenericConstantMaterial
    prop_names = thermal_conductivity
    prop_values = 5
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  nl_abs_tol = 1e-6
  nl_rel_tol = 1e-8
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[VectorPostprocessors]
  [gradient]
    type = PointValueSampler
    points = '0.2 0.2 0
              0.7 0.56 0
              0.4 1 0'
    variable = adjoint
    sort_by = id
  []
[]

[Outputs]
  console = false
  exodus = false
  file_base = 'adjoint'
[]
