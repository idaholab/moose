[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0.0
  xmax = 1.0
  nx = 10
  ymin = 0.0
  ymax = 1.0
  ny = 10
  uniform_refine = 2
  elem_type = QUAD4
[]

[Variables]
  [./u]
  [../]
  [./v]
  [../]
[]

[Functions]
  [./v_left_bc]
    # Left-side boundary condition for v equation, v(0,y,t) = u(0.5,y,t). This is accomplished using a PointValue postprocessor, which is what this input file was designed to test.
    type = ParsedFunction
    value = a
    vals = u_midpoint
    vars = a
  [../]
  [./u_mms_func]
    # MMS Forcing function for the u equation.
    type = ParsedFunction
    value = ' 20*exp(20*t)*x*x*x-6*exp(20*t)*x-(2-0.125*exp(20*t))*sin(5/2*x*pi)-0.125*exp(20*t)-1  
'
  [../]
  [./v_mms_func]
    # MMS forcing function for the v equation.
    type = ParsedFunction
    value = -2.5*exp(20*t)*sin(5/2*x*pi)+2.5*exp(20*t)+25/4*(2-0.125*exp(20*t))*sin(5/2*x*pi)*pi*pi
  [../]
  [./u_right_bc]
    type = ParsedFunction
    value = 3*exp(20*t) # \nabla{u}|_{x=1} = 3\exp(20*t)
  [../]
  [./u_exact]
    # Exact solution for the MMS function for the u variable.
    type = ParsedFunction
    value = exp(20*t)*pow(x,3)+1
  [../]
  [./v_exact]
    # Exact MMS solution for v.
    type = ParsedFunction
    value = (2-0.125*exp(20*t))*sin(5/2*pi*x)+0.125*exp(20*t)+1
  [../]
[]

[Kernels]
  # Strong Form:
  # \frac{\partial u}{\partial t} - \nabla \cdot 0.5 \nabla u - v = 0
  # \frac{\partial u}{\partial t} - \nabla \cdot \nabla v = 0
  # 
  # BCs:
  # u(0,y,t) = 1
  # \nabla u |_{x=1} = 3\exp(20*t)
  # v(0,y,t) = u(0.5,y,t)
  # v(1,y,t) = 3
  # \nabla u |_{y=0,1} = 0
  # \nabla v |_{y=0,1} = 0
  # 
  [./u_time]
    type = TimeDerivative
    variable = u
  [../]
  [./u_diff]
    type = Diffusion
    variable = u
  [../]
  [./u_source]
    type = CoupledForce
    variable = u
    v = v
  [../]
  [./v_diff]
    type = Diffusion
    variable = v
  [../]
  [./u_mms]
    type = UserForcingFunction
    variable = u
    function = u_mms_func
  [../]
  [./v_mms]
    type = UserForcingFunction
    variable = v
    function = v_mms_func
  [../]
  [./v_time]
    type = TimeDerivative
    variable = v
  [../]
[]

[BCs]
  [./u_left]
    type = DirichletBC
    variable = u
    boundary = left # x=0
    value = 1 # u(0,y,t)=1
  [../]
  [./u_right]
    type = FunctionNeumannBC
    variable = u
    boundary = right # x=1
    function = u_right_bc # \nabla{u}|_{x=1}=3\exp(20t)
  [../]
  [./v_left]
    type = FunctionDirichletBC
    variable = v
    boundary = left # x=0
    function = v_left_bc # v(0,y,t) = u(0.5,y,t)
  [../]
  [./v_right]
    type = DirichletBC
    variable = v
    boundary = right # x=1
    value = 3 # v(1,y,t) = 3
  [../]
[]

[Postprocessors]
  [./u_midpoint]
    type = PointValue
    variable = u
    point = '0.5 0.5 0'
  [../]
  [./u_midpoint_exact]
    type = PlotFunction
    function = u_exact
    point = '0.5 0.5 0.0'
  [../]
  [./u_error]
    type = ElementL2Error
    variable = u
    function = u_exact
  [../]
  [./v_error]
    type = ElementL2Error
    variable = v
    function = v_exact
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.01
  petsc_options = snes_mf_operator
  end_time = 0.1
  scheme = crank-nicolson
[]

[Output]
  output_initial = true
  exodus = true
  perf_log = true
[]

[ICs]
  [./u_initial]
    # Use the MMS exact solution to compute the initial conditions.
    function = u_exact
    variable = u
    type = FunctionIC
  [../]
  [./v_exact]
    # Use the MMS exact solution to compute the initial condition.
    function = v_exact
    variable = v
    type = FunctionIC
  [../]
[]

