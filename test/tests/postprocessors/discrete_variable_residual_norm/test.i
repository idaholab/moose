# Heat conduction with fixed temperature on left and convection BC on right:
#
#   d/dx(-k dT/dx) = S'''(T)    (0,1)X(0,1)
#   T = T_inf                    x = 0
#   -k dT/dx = htc (T - T_inf)   x = 1
#
# Source is temperature-dependent:
#   S(T) = B - A * (T - T_inf)^2

k = 15.0
htc = 100.0
T_ambient = 300.0
source_coef_A = 0.1
source_coef_B = 1e4

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    subdomain_name = 'blockA'
  []
  [newblockid]
    type = ParsedSubdomainMeshGenerator
    input = gen
    combinatorial_geometry = 'x > 0.4'
    block_id = 1
    block_name = 'blockB'
  []
[]

!include part_fe.i
!include part_fv.i

[Convergence]
  [nl_conv]
    type = IterationCountConvergence
    max_iterations = 1
    converge_at_max_iterations = true
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  nonlinear_convergence = nl_conv
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  [out]
    type = CSV
    execute_postprocessors_on = 'FINAL'
  []
[]
