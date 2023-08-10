[Mesh]
    [msh]
        type = FileMeshGenerator
        file = '../../FNSF_Blanket.msh'
    []
[]
  
[Outputs]
    exodus = true
    csv = true
[]
  
[Preconditioning]
    [smp]
        type = SMP
        full = true
    []
[]
  
[Executioner]
    type = Steady
    solve_type = 'PJFNK'
    petsc_options_iname = '-pc_type --pc_hypre_type'
    petsc_options_value = 'hypre boomeramg'
[]
  
[Variables]
    [temp]
    []
[]
  
[Kernels]
    [conduction]
        type = HeatConduction
        variable = temp
    []
    [heat]
      type = FNSFHeatSource
      variable = temp
      inner_xi = '-60 -30 -15 15 30 60'
      outer_xi = '-65 -35 -20 20 35 65'
      depth = '1 1.8 1.17'
      heat = '1 1 1 1 1 1 1 1 1 1'
    []
    [force]
        type = BodyForce
        variable = temp
        function = Forced
    []
[]

[Functions]
    [Forced]
        type = ParsedFunction
        value = '3*pi^2*sin(x*pi)*sin(z*pi)*cos(y*pi)-1'
    []
    [Exact]
        type = ParsedFunction
        value = 'sin(x*pi)*sin(z*pi)*cos(y*pi)'
    []
[]
  
[BCs]
    [all]
      type = FunctionDirichletBC
      variable = temp
      boundary = 'front back sides ends'
      function = Exact
    []
[]
  
[Materials]
    [hcm]
      type = HeatConductionMaterial
      specific_heat = 1
      thermal_conductivity = 1
    []
[]

[Postprocessors]
    [error]
        type = ElementL2Error
        function = Exact
        variable = temp
    []
    [h]
        type = AverageElementSize
    []
[]