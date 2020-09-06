[Mesh]
  [msh]
    type = GeneratedMeshGenerator
    nx = 4
    ny = 4
    xmin = -1
    xmax = 1
    ymin = -1
    ymax = 1
    dim = 2
  []
  [subdomain_1]
    type = SubdomainBoundingBoxGenerator
    input = msh
    bottom_left = '-1 -1 0'
    top_right = '0 0 0'
    block_id = 1
  []
  [subdomain_2]
    type = SubdomainBoundingBoxGenerator
    input = subdomain_1
    bottom_left = '-1 0 0'
    top_right = '0 1 0'
    block_id = 2
  []
  [subdomain_3]
    type = SubdomainBoundingBoxGenerator
    input = subdomain_2
    bottom_left = '0 -1 0'
    top_right = '1 0 0'
    block_id = 3
  []
  [subdomain_4]
    type = SubdomainBoundingBoxGenerator
    input = subdomain_3
    bottom_left = '0 0 0'
    top_right = '1 1 0'
    block_id = 4
  []
  [split]
    type = BreakMeshByBlockGenerator
    input = subdomain_4
  []
  [node_set1]
    input = split
    type = ExtraNodesetGenerator
    coord = '-0.5 0.5 0'
    new_boundary = fix_all
  []
  [node_set2]
    input = node_set1
    type = ExtraNodesetGenerator
    coord = '0.5 0.5 0'
    new_boundary = fix_x
  []
  [sidesets]
    type = SideSetsFromNormalsGenerator
    input = node_set2
    normals = '1  0  0
              -1  0  0
               0  1  0
               0 -1  0'
    fixed_normal = true
    new_boundary = 'x1 x0 y1 y0'
  []
  [node_move]
    input = sidesets
    type = ExtraNodesetGenerator
    coord = '-0.5 -0.5 0'
    new_boundary = node_move
  []
  [break_boundary]
    input = node_move
    type = BreakBoundaryOnSubdomainGenerator
    boundaries = 'x1 x0 y1 y0'
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Modules/TensorMechanics/Master]
  [all]
    strain = SMALL
    add_variables = true
    generate_output = 'stress_xx stress_yy stress_xy'
  []
[]

[Modules/TensorMechanics/CohesiveZoneMaster]
  inactive=''
  [./czm1]
    boundary = 'interface'
    displacements = 'disp_x disp_y'
  [../]
[]


[InterfaceKernels]
  inactive=''
  [czmx]
    type = CZMInterfaceKernel
    variable =disp_x
    neighbor_var = disp_x
    component = 0
    boundary = interface
  []
  [czmy]
    type = CZMInterfaceKernel
    variable =disp_y
    neighbor_var = disp_y
    component = 1
    boundary = interface
  []
[]


[BCs]
  [Periodic]
    inactive=''
    [x_x]
      variable = disp_x
      primary  = x0
      secondary = x1
      translation = '2 0 0'
    []
    [y_x]
      variable = disp_y
      primary  = x0
      secondary = x1
      translation = '2 0 0'
    []
    [x_y]
      variable = disp_x
      primary  = y0
      secondary = y1
      translation = '0 2 0'
    []
    [y_y]
      variable = disp_y
      primary  = y0
      secondary = y1
      translation = '0 2 0'
    []
    [x_x_1]
      variable = disp_x
      primary  = x0_to_1
      secondary = x1_to_3
      translation = '2 0 0'
    []
    [y_x_1]
      variable = disp_y
      primary  = x0_to_1
      secondary = x1_to_3
      translation = '2 0 0'
    []
    [x_x_3]
      variable = disp_x
      primary  = x0_to_2
      secondary = x1_to_4
      translation = '2 0 0'
    []
    [y_x_3]
      variable = disp_y
      primary  = x0_to_2
      secondary = x1_to_4
      translation = '2 0 0'
    []
    [x_y_1]
      variable = disp_x
      primary  = y0_to_1
      secondary = y1_to_2
      translation = '0 2 0'
    []
    [y_y_1]
      variable = disp_y
      primary  = y0_to_1
      secondary = y1_to_2
      translation = '0 2 0'
    []
    [x_y_3]
      variable = disp_x
      primary  = y0_to_3
      secondary = y1_to_4
      translation = '0 2 0'
    []
    [y_y_3]
      variable = disp_y
      primary  = y0_to_3
      secondary = y1_to_4
      translation = '0 2 0'
    []
  []


  [displacement_yt]
    type = FunctionDirichletBC
    boundary = node_move
    variable = disp_y
    function = '0.1*t'
  []

  [fix1_x]
    type = DirichletBC
    boundary = "fix_all"
    variable = disp_x
    value = 0
  []
  [fix1_y]
    type = DirichletBC
    boundary = "fix_all"
    variable = disp_y
    value = 0
  []

  [fix2_x]
    type = DirichletBC
    boundary = "fix_x"
    variable = disp_x
    value = 0
  []
[]

[Materials]
  [Elasticity_tensor]
    type = ComputeElasticityTensor
    block = '1'
    fill_method = symmetric_isotropic
    C_ijkl = '0.3 0.5e8'
  []
  [Elasticity_tensor2]
    type = ComputeElasticityTensor
    block = '2'
    fill_method = symmetric_isotropic
    C_ijkl = '0.3 0.5e7'
  []
  [Elasticity_tensor3]
    type = ComputeElasticityTensor
    block = '3'
    fill_method = symmetric_isotropic
    C_ijkl = '0.3 1e8'
  []
  [Elasticity_tensor4]
    type = ComputeElasticityTensor
    block = '4'
    fill_method = symmetric_isotropic
    C_ijkl = '0.3 0.5e9'
  []
  [stress]
    type = ComputeLinearElasticStress
    block = '1 2 3 4'
  []
  [czm_3dc]
    type = SalehaniIrani3DCTraction
    boundary = 'interface'
    normal_gap_at_maximum_normal_traction = 1
    tangential_gap_at_maximum_shear_traction = 0.5
    maximum_normal_traction = 1000
    maximum_shear_traction = 700
    displacements = 'disp_x disp_y'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  [s11]
    type = ElementAverageValue
    variable = stress_xx
    execute_on = 'initial timestep_end'
  []
  [s12]
    type = ElementAverageValue
    variable = stress_xy
    execute_on = 'initial timestep_end'
  []
  [s22]
    type = ElementAverageValue
    variable = stress_yy
    execute_on = 'initial timestep_end'
  []
[]

[Functions]
  [tol_fcn]
    type = ConstantFunction
    value = 1e-10
  []

  [should_be_zero0_fcn]
    type = ParsedFunction
    value = 'if(abs(blx-brx)<tol_fcn,0,1) + if(abs(blx-tlx)<tol_fcn,0,1) + if(abs(blx-trx)<tol_fcn,0,1) + if(abs(brx-tlx)<tol_fcn,0,1) + if(abs(brx-trx)<tol_fcn,0,1) + if(abs(tlx-trx)<tol_fcn,0,1)'
    vars = 'blx brx tlx trx tol_fcn'
    vals = 'bl_x br_x tl_x tr_x tol_fcn'
  []
  [should_be_zero1_fcn]
    type = ParsedFunction
    value = 'if(abs(bly-bry)<tol_fcn,0,1) + if(abs(bly-tly)<tol_fcn,0,1) + if(abs(bly-try)<tol_fcn,0,1) + if(abs(bry-tly)<tol_fcn,0,1) + if(abs(bry-try)<tol_fcn,0,1) + if(abs(tly-try)<tol_fcn,0,1)'
    vars = 'bly bry tly try tol_fcn'
    vals = 'bl_y br_y tl_y tr_y tol_fcn'
  []

  [should_be_zero2_fcn]
    type = ParsedFunction
    value = 'if(abs(tclx-bclx)<tol_fcn,0,1)'
    vars = 'tclx bclx tol_fcn'
    vals = 'tcl_x bcl_x tol_fcn'
  []
  [should_be_zero3_fcn]
    type = ParsedFunction
    value = 'if(abs(tcly-bcly)<tol_fcn,0,1)'
    vars = 'tcly bcly tol_fcn'
    vals = 'tcl_y bcl_y tol_fcn'
  []

  [should_be_zero4_fcn]
    type = ParsedFunction
    value = 'if(abs(tcrx-bcrx)<tol_fcn,0,1)'
    vars = 'tcrx bcrx tol_fcn'
    vals = 'tcr_x bcr_x tol_fcn'
  []
  [should_be_zero5_fcn]
    type = ParsedFunction
    value = 'if(abs(tcry-bcry)<tol_fcn,0,1)'
    vars = 'tcry bcry tol_fcn'
    vals = 'tcr_y bcr_y tol_fcn'
  []

  [should_be_zero6_fcn]
    type = ParsedFunction
    value = 'if(abs(lctx-rctx)<tol_fcn,0,1)'
    vars = 'lctx rctx tol_fcn'
    vals = 'lct_x rct_x tol_fcn'
  []
  [should_be_zero7_fcn]
    type = ParsedFunction
    value = 'if(abs(lcty-rcty)<tol_fcn,0,1)'
    vars = 'lcty rcty tol_fcn'
    vals = 'lct_y rct_y tol_fcn'
  []

  [should_be_zero8_fcn]
    type = ParsedFunction
    value = 'if(abs(lcbx-rcbx)<tol_fcn,0,1)'
    vars = 'lcbx rcbx tol_fcn'
    vals = 'lcb_x rcb_x tol_fcn'
  []
  [should_be_zero9_fcn]
    type = ParsedFunction
    value = 'if(abs(lcby-rcby)<tol_fcn,0,1)'
    vars = 'lcby rcby tol_fcn'
    vals = 'lcb_y rcb_y tol_fcn'
  []
[]


[Executioner]
  type = Transient

  solve_type = 'newton'
  line_search = none

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'

  l_max_its = 2
  l_tol = 1e-14
  nl_max_its = 10
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10

  start_time = 0.0
  dt = 0.2
  dtmin = 0.2
  end_time = 0.2
[]

[Postprocessors]
  # corner nodes
  [bl_x]
    type = NodalVariableValue
    variable = disp_x
    nodeid = 0
  []
  [bl_y]
    type = NodalVariableValue
    variable = disp_y
    nodeid = 0
  []
  [br_x]
    type = NodalVariableValue
    variable = disp_x
    nodeid = 8
  []
  [br_y]
    type = NodalVariableValue
    variable = disp_y
    nodeid = 8
  []
  [tl_x]
    type = NodalVariableValue
    variable = disp_x
    nodeid = 21
  []
  [tl_y]
    type = NodalVariableValue
    variable = disp_y
    nodeid = 21
  []
  [tr_x]
    type = NodalVariableValue
    variable = disp_x
    nodeid = 24
  []
  [tr_y]
    type = NodalVariableValue
    variable = disp_y
    nodeid = 24
  []

    # center nodes top bottom plane
  [bcl_x]
    type = NodalVariableValue
    variable = disp_x
    nodeid = 4
  []
  [bcl_y]
    type = NodalVariableValue
    variable = disp_y
    nodeid = 4
  []
  [tcl_x]
    type = NodalVariableValue
    variable = disp_x
    nodeid = 22
  []
  [tcl_y]
    type = NodalVariableValue
    variable = disp_y
    nodeid = 22
  []
  [bcr_x]
    type = NodalVariableValue
    variable = disp_x
    nodeid = 25
  []
  [bcr_y]
    type = NodalVariableValue
    variable = disp_y
    nodeid = 25
  []
  [tcr_x]
    type = NodalVariableValue
    variable = disp_x
    nodeid = 35
  []
  [tcr_y]
    type = NodalVariableValue
    variable = disp_y
    nodeid = 35
  []

  # center nodes left right plane
  [lct_x]
    type = NodalVariableValue
    variable = disp_x
    nodeid = 28
  []
  [lct_y]
    type = NodalVariableValue
    variable = disp_y
    nodeid = 28
  []
  [rct_x]
    type = NodalVariableValue
    variable = disp_x
    nodeid = 33
  []
  [rct_y]
    type = NodalVariableValue
    variable = disp_y
    nodeid = 33
  []

  [lcb_x]
    type = NodalVariableValue
    variable = disp_x
    nodeid = 11
  []
  [lcb_y]
    type = NodalVariableValue
    variable = disp_y
    nodeid = 11
  []
  [rcb_x]
    type = NodalVariableValue
    variable = disp_x
    nodeid = 14
  []
  [rcb_y]
    type = NodalVariableValue
    variable = disp_y
    nodeid = 14
  []

  [should_be_zero_corners_x]
    type = FunctionValuePostprocessor
    function = should_be_zero0_fcn
  []
  [should_be_zero_corners_y]
    type = FunctionValuePostprocessor
    function = should_be_zero1_fcn
  []
  [should_be_zero_tbcl_x]
    type = FunctionValuePostprocessor
    function = should_be_zero2_fcn
  []
  [should_be_zero_tbcl_y]
    type = FunctionValuePostprocessor
    function = should_be_zero3_fcn
  []
  [should_be_zero_tbcr_x]
    type = FunctionValuePostprocessor
    function = should_be_zero4_fcn
  []
  [should_be_zero_tbcr_y]
    type = FunctionValuePostprocessor
    function = should_be_zero5_fcn
  []

  [should_be_zero_lrct_x]
    type = FunctionValuePostprocessor
    function = should_be_zero6_fcn
  []
  [should_be_zero_lrct_y]
    type = FunctionValuePostprocessor
    function = should_be_zero7_fcn
  []
  [should_be_zero_lrcb_x]
    type = FunctionValuePostprocessor
    function = should_be_zero8_fcn
  []
  [should_be_zero_lrcb_y]
    type = FunctionValuePostprocessor
    function = should_be_zero9_fcn
  []
[]

[Outputs]
  exodus = true
[]
