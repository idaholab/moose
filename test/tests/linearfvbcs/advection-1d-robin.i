##################################################################
c     = 0.1 # advection veclocity (+x direction)
amp   = 1.0 # sinusoid amplitude, for u_exact

#x_l   = ${fparse 0.1*pi} # domain bound (left)
#x_r   = ${fparse pi}     # domain bound (right)

x_l   = ${fparse 0.0*pi} # domain bound (left)
x_r   = ${fparse 0.9*pi}     # domain bound (right)

alpha = 0.001 # robin BC coeff for gradient term
beta  = 0.999 # robin BC coeff for variable term
#gamma = ${fparse (-alpha*amp*sin(x_l))+(beta*amp*cos(x_l))} # RHS of Robin BC, applied at left boundary
gamma = ${fparse (-alpha*amp*sin(x_r))+(beta*amp*cos(x_r))} # RHS of Robin BC, applied at right boundary

npts = 1000
##################################################################

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim  = 1
    nx   = ${fparse npts}
    xmin = ${fparse x_l}
    xmax = ${fparse x_r}
  []
[]

[Problem]
  linear_sys_names = 'u_sys'
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    solver_sys = 'u_sys'
    initial_condition = 0.0
  []
[]

[Functions]
  [u_exact]
    type = ParsedFunction
    expression = '${fparse amp}*cos(x)'
  []
  [source_fn]
    type = ParsedFunction
    expression = '${fparse -c*amp}*sin(x)'
  []
[]

[LinearFVKernels]
  [advection]
    type = LinearFVAdvection
    variable = u
    velocity = "${fparse c} 0 0"
    advected_interp_method = upwind
  []
  [source]
    type = LinearFVSource
    variable = u
    source_density = source_fn
  []
[]

[LinearFVBCs]
#  [dir_r]
#    type = LinearFVAdvectionDiffusionFunctorDirichletBC
#    variable = u
#    boundary = "right"
#    functor = 0
#  []
  [outflow]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = u
    boundary = "right"
    use_two_term_expansion = false
  []
  [rob_l]
    type = LinearFVAdvectionDiffusionFunctorRobinBC
    variable = u
    boundary = "left"
    alpha = ${fparse alpha}
    beta  = ${fparse beta} 
    gamma = ${fparse gamma}
  []
#  [rob_l]
#    type = LinearFVAdvectionDiffusionFunctorDirichletBC
#    variable = u
#    boundary = "left"
#    functor = u_exact
#  []
[]

[Postprocessors]
  [h]
    type = AverageElementSize
#    execute_on = FINAL
  []
  [error]
    type = ElementL2FunctorError
    approximate = u
    exact = u_exact
#    execute_on = FINAL
  []
  [recv]
    type = ConstantPostprocessor
    value = ${fparse gamma}
  []
[]

[VectorPostprocessors]
  [u]
    type = LineValueSampler
    start_point = '${fparse x_l} 0 0'
    end_point = '${fparse x_r} 0 0'
    sort_by = x
    num_points = ${fparse npts} 
    variable = 'u'
  []
[]

[Executioner]
  type = Steady
  system_names = u_sys
  l_tol = 1e-10
#  petsc_options_iname = '-pc_type'
#  petsc_options_value = 'lu'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  [csv]
    type = CSV
#    execute_on = FINAL
  []
  [exodus]
    type = Exodus
#    execute_on = FINAL
  []
[]
