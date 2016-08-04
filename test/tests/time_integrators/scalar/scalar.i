[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 1
  ny = 1
  elem_type = QUAD4
[]

[Variables]
  [./n]
    family = SCALAR
    order = FIRST
    initial_condition = 1
  [../]
[]

[ScalarKernels]
  [./dn]
    type = ODETimeDerivative
    variable = n
  [../]
  [./ode1]
    type = ParsedODEKernel
    function = '-n'
    variable = n
    # implicit = false
  [../]
[]

[Executioner]
  type = Transient
  [./TimeIntegrator]
    # type = ImplicitEuler
    # type = BDF2
    type = CrankNicolson
    # type = LStableDirk2
    # type = LStableDirk3
    # type = LStableDirk4
    # type = AStableDirk4
    #
    # Note: these may be affected by ScalarKernel not having an
    # implicit=false flag.
    # type = ExplicitEuler
    # type = ExplicitMidpoint
    # type = Heun
    # type = Ralston
  [../]
  # line_search = 'NONE'
  start_time = 0
  end_time = 1
  dt = 0.001
  dtmin = 0.001 # Don't allow timestep cutting
  solve_type = 'PJFNK'
  nl_max_its = 2
  nl_abs_tol = 1.e-12 # This is an ODE, so nl_abs_tol makes sense.
[]

[Outputs]
  csv = true
[]


# Solution at time t=1 for different methods.  The exact solution is:
# u = exp(1) ~ 2.718281828459045235360287
#
# dt=.016
# ImplicitEuler:    2.7402628637568
# BDF2:             2.7190269112671
# CrankNicolson:    2.7183394714927
# LStableDirk2:     2.7183097611264
# LStableDirk3:     2.718281539505
# LStableDirk4:     2.718281828304
# AStableDirk4:     2.7182818568402
# ExplicitEuler:    2.6969346589203
# ExplicitMidpoint: 2.7181679190129
# Heun:             2.7181679198788
# Ralston:          2.7181679207473
#
# dt=.008
# ImplicitEuler:    2.729235306395
# BDF2:             2.7184692784995
# CrankNicolson:    2.7182963274244
# LStableDirk2:     2.7182888593142
# LStableDirk3:     2.7182817920991
# LStableDirk4:     2.7182818284544
# AStableDirk4:     2.7182818285551
# ExplicitEuler:    2.707487833642
# ExplicitMidpoint: 2.7182530071624
# Heun:             2.7182530070178
# Ralston:          2.7182530069651
#
# dt=.004
# ImplicitEuler:    2.7237384025342
# BDF2:             2.7183288153879
# CrankNicolson:    2.7182854527347
# LStableDirk2:     2.7182835860427
# LStableDirk3:     2.7182818237262
# LStableDirk4:     2.7182818284577
# AStableDirk4:     2.7182818279616
# ExplicitEuler:    2.7128651224071
# ExplicitMidpoint: 2.7182746008703
# Heun:             2.7182746014289
# Ralston:          2.718274601552
#
# dt=.002
# ImplicitEuler:    2.7210051030737
# BDF2:             2.718293591276
# CrankNicolson:    2.7182827355351
# LStableDirk2:     2.7182822679275
# LStableDirk3:     2.7182818278485
# LStableDirk4:     2.718281828459
# AStableDirk4:     2.7182818280332
# ExplicitEuler:    2.7155685208978
# ExplicitMidpoint: 2.7182800187281
# Heun:             2.7182800189882
# Ralston:          2.718280018968
#
# dt=.001
# ImplicitEuler:    2.7196422163254
# BDF2:             2.7182847724051
# CrankNicolson:    2.718282054835
# LStableDirk2:     2.7182819381549
# LStableDirk3:     2.7182818284058
# LStableDirk4:     2.7182818284591
# AStableDirk4:     2.7182818284201
# ExplicitEuler:    2.716923932128
# ExplicitMidpoint: 2.7182813756827
# Heun:             2.7182813757517
# Ralston:          2.7182813756824
