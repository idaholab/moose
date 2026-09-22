# Linear elasticity cantilever, libMesh backend, for comparison against mfem_cantilever.i.
#
# An 8x1x1 beam of HEX8 elements fixed on 'left' and loaded downward on 'right', solved with PCG
# and BoomerAMG. 'refinement' controls the mesh: the paired MFEM input refines its coarse mesh the
# same number of times, so both backends discretize the same problem with the same number of
# degrees of freedom. The reference measurements use refinement = 5 (262144 elements, 839619 DoFs)
# on 24 MPI ranks.
#
# The mesh is generated directly at its final size, and a single LAGRANGE_VEC variable carries all
# three displacement components, so that the operator matches MFEM's ElasticityIntegrator on a
# vector H1 space.

refinement = 3

[Mesh]
  [gen_mesh]
    type = GeneratedMeshGenerator
    dim = 3
    # fparse evaluates 2^n in floating point, which is not exactly integral for
    # every n, so the counts are rounded before they reach an unsigned int parameter
    xmax = 8
    ymax = 1
    zmax = 1
    nx = ${fparse int(8 * 2^refinement)}
    ny = ${fparse int(2^refinement)}
    nz = ${fparse int(2^refinement)}
  []
  use_displaced_mesh = false
[]

[Variables]
  [disp]
    family = LAGRANGE_VEC
    order = FIRST
  []
[]

[Kernels]
  [elasticity]
    type = KokkosLinearElasticity
    variable = disp
    lambda = lambda
    mu = mu
  []
[]

[Materials]
  [lame]
    type = KokkosGenericConstantMaterial
    prop_names = 'lambda mu'
    prop_values = '60.5e9 25.9e9'
  []
[]

[BCs]
  [fixed]
    type = KokkosVectorDirichletBC
    variable = disp
    boundary = 'left'
    values = '0 0 0'
  []
  [load]
    type = KokkosVectorNeumannBC
    variable = disp
    boundary = 'right'
    values = '0 0 -1.0e-2'
  []
[]

[Executioner]
  type = Steady
  solve_type = LINEAR
  # PCHYPRE leaves P_max at 0, which lets ext+i interpolation build coarse operators denser than
  # the fine one. mfem::HypreBoomerAMG truncates to four entries per row by default, so setting it
  # here is what makes the two backends precondition the same operator the same way. One level of
  # aggressive coarsening is cheaper still on this problem and is requested of both backends, so
  # that they continue to precondition the operator the same way.
  # The Kokkos assembly fills the Jacobian through PETSc's COO interface, which MATHYPRE
  # implements, so asking for that matrix type has the assembly build the hypre ParCSR that
  # BoomerAMG consumes. PCSetUp then skips the AIJ-to-ParCSR conversion, and the Krylov iterations
  # use hypre's ParCSR matrix-vector product, which is measurably faster than PETSc's MPIAIJ one on
  # this operator.
  petsc_options_iname = '-ksp_type -pc_type  -pc_hypre_type  -pc_hypre_boomeramg_coarsen_type  -pc_hypre_boomeramg_interp_type -pc_hypre_boomeramg_strong_threshold -pc_hypre_boomeramg_numfunctions -pc_hypre_boomeramg_P_max -pc_hypre_boomeramg_agg_nl -nl0_mat_type'
  petsc_options_value = ' cg       hypre     boomeramg       HMIS                              ext+i                           0.7                                   3                                4                         1                        hypre'
  l_tol = 1e-8
  l_max_its = 500
[]

[AuxVariables]
  [disp_z]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxKernels]
  [disp_z]
    type = VectorVariableComponentAux
    variable = disp_z
    vector_variable = disp
    component = z
  []
[]

[Postprocessors]
  [maxZ]
    type = ElementExtremeValue
    variable = disp_z
    value_type = max_abs
  []
[]

[Outputs]
  print_linear_residuals = false
  csv = true
  [perf]
    type = PerfGraphOutput
    level = 5
    heaviest_sections = 12
  []
[]
