[Mesh]
  [msh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
  []
  [bottom_left]
    type = ExtraNodesetGenerator
    input = msh
    new_boundary = 'bottom_left'
    coord = '0 0 0'
  []
  [top_left]
    type = ExtraNodesetGenerator
    input = bottom_left
    new_boundary = 'top_left'
    coord = '0 1 0'
  []
  [top_right]
    type = ExtraNodesetGenerator
    input = top_left
    new_boundary = 'top_right'
    coord = '1 1 0'
  []
  [bottom_right]
    type = ExtraNodesetGenerator
    input = top_right
    new_boundary = 'bottom_right'
    coord = '1 0 0'
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
  large_kinematics = true
[]

[Variables]
  [disp_x]
  []
  [disp_y]
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
[]

[AuxVariables]
  [stress_xx]
    order = CONSTANT
    family = MONOMIAL
    [AuxKernel]
      type = RankTwoAux
      rank_two_tensor = cauchy_stress
      index_i = 0
      index_j = 0
      execute_on = TIMESTEP_END
    []
  []
  [stress_yy]
    order = CONSTANT
    family = MONOMIAL
    [AuxKernel]
      type = RankTwoAux
      rank_two_tensor = cauchy_stress
      index_i = 1
      index_j = 1
      execute_on = TIMESTEP_END
    []
  []
[]

[BCs]
  [fix_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom_left'
    value = 0
  []
  [fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'bottom_left'
    value = 0
  []
  [top_left_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'top_left'
    function = 'theta:=if(t<1,0,t-1); -sin(theta)'
  []
  [top_left_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'top_left'
    function = 'theta:=if(t<1,0,t-1); cos(theta)-1'
  []
  [bottom_right_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'bottom_right'
    function = 'theta:=if(t<1,0,t-1); if(t<1,t,2*cos(theta)-1)'
  []
  [bottom_right_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'bottom_right'
    function = 'theta:=if(t<1,0,t-1); if(t<1,0,2*sin(theta))'
  []
  [top_right_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'top_right'
    function = 'theta:=if(t<1,0,t-1); phi:=theta+atan(0.5); if(t<1,t,sqrt(5)*cos(phi)-1)'
  []
  [top_right_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'top_right'
    function = 'theta:=if(t<1,0,t-1); phi:=theta+atan(0.5); if(t<1,0,sqrt(5)*sin(phi)-1)'
  []
[]

[Materials]
  [elastic_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e5
    poissons_ratio = 0
  []
  [stress]
    type = ComputeLagrangianLinearElasticStress
  []
  [strain]
    type = ComputeLagrangianStrain
  []
[]

[Postprocessors]
  [sxx]
    type = ElementAverageValue
    variable = stress_xx
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [syy]
    type = ElementAverageValue
    variable = stress_yy
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  dt = 0.01
  end_time = '${fparse pi/2+1}'

  solve_type = NEWTON
  line_search = none

  petsc_options_iname = -pc_type
  petsc_options_value = lu
  automatic_scaling = true

  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-10
[]

[Outputs]
  csv = true
[]
