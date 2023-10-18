power = 4

E0 = 1.0e-6
E1 = 0.2
E2 = 0.6
E3 = 1.0

rho0 = 1.0e-6
rho1 = 0.4
rho2 = 0.7
rho3 = 1.0

C0 = 1.0e-6
C1 = 0.5
C2 = 0.8
C3 = 1.0

[Problem]
  solve = false
[]

[Mesh]
  [MeshGenerator]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
  []
[]

[AuxVariables]
  [mat_den]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [mat_den]
    type = FunctionAux
    variable = mat_den
    function = mat_den_fn
  []
[]

[Functions]
  [mat_den_fn]
    type = ParsedFunction
    expression = .01*t
  []
[]
[Materials]
  [E_phys]
    type = DerivativeParsedMaterial
    # ordered multimaterial simp
    expression = "A1:=(${E0}-${E1})/(${rho0}^${power}-${rho1}^${power}); "
                 "B1:=${E0}-A1*${rho0}^${power}; E1:=A1*mat_den^${power}+B1; "
                 "A2:=(${E1}-${E2})/(${rho1}^${power}-${rho2}^${power}); "
                 "B2:=${E1}-A2*${rho1}^${power}; E2:=A2*mat_den^${power}+B2; "
                 "A3:=(${E2}-${E3})/(${rho2}^${power}-${rho3}^${power}); "
                 "B3:=${E2}-A3*${rho2}^${power}; E3:=A3*mat_den^${power}+B3; "
                 "if(mat_den<${rho1},E1,if(mat_den<${rho2},E2,E3))"
    coupled_variables = 'mat_den'
    property_name = E_phys
  []

  [Cost_mat]
    type = DerivativeParsedMaterial
    # ordered multimaterial simp
    expression = "A1:=(${C0}-${C1})/(${rho0}^(1/${power})-${rho1}^(1/${power})); "
                 "B1:=${C0}-A1*${rho0}^(1/${power}); C1:=A1*mat_den^(1/${power})+B1; "
                 "A2:=(${C1}-${C2})/(${rho1}^(1/${power})-${rho2}^(1/${power})); "
                 "B2:=${C1}-A2*${rho1}^(1/${power}); C2:=A2*mat_den^(1/${power})+B2; "
                 "A3:=(${C2}-${C3})/(${rho2}^(1/${power})-${rho3}^(1/${power})); "
                 "B3:=${C2}-A3*${rho2}^(1/${power}); C3:=A3*mat_den^(1/${power})+B3; "
                 "if(mat_den<${rho1},C1,if(mat_den<${rho2},C2,C3))"
    coupled_variables = 'mat_den'
    property_name = Cost_mat
  []
[]
[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'
  nl_abs_tol = 1e-8
  dt = 1.0
  num_steps = 100
[]

[Outputs]
  csv = true
  print_linear_residuals = false
[]

[Postprocessors]
  [mat_den]
    type = PointValue
    point = '0.5 0.5 0'
    variable = mat_den
  []
  [E_phys]
    type = ElementExtremeMaterialProperty
    mat_prop = E_phys
    value_type = max
  []
  [Cost_mat]
    type = ElementExtremeMaterialProperty
    mat_prop = Cost_mat
    value_type = max
  []
[]
