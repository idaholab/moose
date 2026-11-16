# One HEX8 with prescribed displacements that give a different det(F) at each quadrature point.
# With F-bar stabilization the Cauchy stress at each quadrature point is the Neo-Hookean stress
# of the stabilized deformation gradient Fbar, sigma = Fbar S(Fbar) Fbar^T / det(Fbar). The
# output lists sigma_xx per quadrature point (the stress error of using F instead of Fbar is a
# scalar factor, so one component shows it).
[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  large_kinematics = true
  stabilize_strain = true
[]

[Mesh]
  [msh]
    type = GeneratedMeshGenerator
    dim = 3
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[ICs]
  [x]
    type = FunctionIC
    variable = disp_x
    function = '0.3*x*y + 0.1*z'
  []
  [y]
    type = FunctionIC
    variable = disp_y
    function = '0.2*y*z - 0.15*x'
  []
  [z]
    type = FunctionIC
    variable = disp_z
    function = '0.25*x*z + 0.05*y'
  []
[]

[Kernels]
  [sdx]
    type = TotalLagrangianStressDivergence
    variable = disp_x
    component = 0
  []
  [sdy]
    type = TotalLagrangianStressDivergence
    variable = disp_y
    component = 1
  []
  [sdz]
    type = TotalLagrangianStressDivergence
    variable = disp_z
    component = 2
  []
[]

[Materials]
  [stress]
    type = ComputeNeoHookeanStress
    lambda = 4000.0
    mu = 6700.0
  []
  [strain]
    type = ComputeLagrangianStrain
  []
  [s_xx]
    type = RankTwoCartesianComponent
    rank_two_tensor = cauchy_stress
    property_name = s_xx
    index_i = 0
    index_j = 0
  []
[]

[VectorPostprocessors]
  [stress]
    type = ElementMaterialSampler
    property = 's_xx'
  []
[]

[Executioner]
  # Evaluate the stress at the prescribed displacements, without solving
  type = Steady
  solve_type = NEWTON
  nl_max_its = 0
  nl_abs_tol = 1e100
[]

[Outputs]
  csv = true
[]
