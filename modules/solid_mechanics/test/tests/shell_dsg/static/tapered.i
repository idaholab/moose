# Tapered strip stretched in its own plane, DSG shell elements.
#
# A tapered strip in the XY plane with E = 210000, nu = 0.3 and thickness 0.1
# is held in X at its left end and in Y at the middle node of that end, and
# stretched by a uniform displacement of 0.2 at its right end. The problem is
# symmetric about the axis of the strip and so are the results. The mesh file
# is the one of the module's shell battery: block 1, boundary 2 the left end,
# boundary 10 its middle node and boundary 3 the right end.

[Mesh]
  [input]
    type = FileMeshGenerator
    file = taperedmesh.e
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
  [rot_x]
  []
  [rot_y]
  []
  [rot_z]
  []
[]

[AuxVariables]
  [membrane_force_11]
    order = CONSTANT
    family = MONOMIAL
  []
  [membrane_force_22]
    order = CONSTANT
    family = MONOMIAL
  []
  [membrane_force_12]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [membrane_force_11]
    type = DSGShellResultantsAux
    variable = membrane_force_11
    resultant = membrane_force_11
    execute_on = TIMESTEP_END
  []
  [membrane_force_22]
    type = DSGShellResultantsAux
    variable = membrane_force_22
    resultant = membrane_force_22
    execute_on = TIMESTEP_END
  []
  [membrane_force_12]
    type = DSGShellResultantsAux
    variable = membrane_force_12
    resultant = membrane_force_12
    execute_on = TIMESTEP_END
  []
[]

[BCs]
  [hold_x]
    type = DirichletBC
    variable = disp_x
    value = 0.0
    boundary = '2'
  []
  [hold_y]
    type = DirichletBC
    variable = disp_y
    value = 0.0
    boundary = 10
  []
  [stretch]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = '3'
    function = displacement
  []
[]

[Functions]
  [displacement]
    type = PiecewiseLinear
    x = '0.0 1.0'
    y = '0.0 0.2'
  []
[]

[Kernels]
  [solid_disp_x]
    type = StressDivergenceDSGShell
    component = 0
    variable = disp_x
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [solid_disp_y]
    type = StressDivergenceDSGShell
    component = 1
    variable = disp_y
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [solid_disp_z]
    type = StressDivergenceDSGShell
    component = 2
    variable = disp_z
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [solid_rot_x]
    type = StressDivergenceDSGShell
    component = 3
    variable = rot_x
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [solid_rot_y]
    type = StressDivergenceDSGShell
    component = 4
    variable = rot_y
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [solid_rot_z]
    type = StressDivergenceDSGShell
    component = 5
    variable = rot_z
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
[]

[Materials]
  [strain]
    type = ComputeDSGShellStrain
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [section]
    type = ComputeDSGShellSectionStiffness
    youngs_modulus = 210000
    poissons_ratio = 0.3
    thickness = 0.1
  []
  [resultants]
    type = ComputeDSGShellResultants
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = 'none'
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8
  dt = 1.0
  dtmin = 1.0
  end_time = 1.0
[]

[Postprocessors]
  [membrane_force_11_max]
    type = ElementExtremeValue
    variable = membrane_force_11
    value_type = max
  []
  [membrane_force_11_min]
    type = ElementExtremeValue
    variable = membrane_force_11
    value_type = min
  []
  [membrane_force_12_max]
    type = ElementExtremeValue
    variable = membrane_force_12
    value_type = max
  []
  [disp_y_max]
    type = NodalExtremeValue
    variable = disp_y
    value_type = max
  []
[]

[Outputs]
  csv = true
[]
