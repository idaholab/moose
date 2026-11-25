# Natural frequencies of the cantilever plate of free_vibration.i by an eigenvalue solve:
# the plate of 1.0 m by 1.5 m, E = 2.1e6, nu = 0.3, thickness 0.1 and density 1, clamped
# along one short edge, on 4 x 8 elements.
#
# The stiffness kernels are the ones of a static analysis. The mass kernels carry the
# eigen vector tag, every clamped variable carries an EigenDirichletBC beside its
# DirichletBC, and the preconditioner assembles the full coupling between the
# displacements and the rotations, without which the rotations would be absent from the
# eigenvectors. The rotations have no mass on the drilling axis, so the mass matrix is
# singular; the shift-and-invert transformation handles that.
#
# The thin plate value of Blevins, "Formulas for Dynamics, Acoustics and Vibration"
# (Wiley, 2016), Table 5.3 case 11, is 10.785 Hz for the first mode, omega^2 = 4592 s^-2.
# The shear deformation and the rotary inertia of a plate of this thickness lower it;
# the transient of free_vibration.i reads 10.587 Hz on this mesh, which the period
# elongation of the Newmark scheme at its time step puts 0.9 percent below the discrete
# frequency of the same mesh.

[Mesh]
  [plate]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 8
    xmin = 0.0
    xmax = 1.0
    ymin = 0.0
    ymax = 1.5
    elem_type = QUAD4
  []
  # No rotation here; the test specification sets the Euler angles of this generator to run
  # the same plate inclined in space.
  [orientation]
    type = TransformGenerator
    input = plate
    transform = ROTATE
    vector_value = '0 0 0'
  []
[]

[Variables]
  [disp_x]
    order = FIRST
    family = LAGRANGE
  []
  [disp_y]
    order = FIRST
    family = LAGRANGE
  []
  [disp_z]
    order = FIRST
    family = LAGRANGE
  []
  [rot_x]
    order = FIRST
    family = LAGRANGE
  []
  [rot_y]
    order = FIRST
    family = LAGRANGE
  []
  [rot_z]
    order = FIRST
    family = LAGRANGE
  []
[]

[BCs]
  [eigen_clamp_disp_x]
    type = EigenDirichletBC
    variable = disp_x
    boundary = bottom
  []
  [eigen_clamp_disp_y]
    type = EigenDirichletBC
    variable = disp_y
    boundary = bottom
  []
  [eigen_clamp_disp_z]
    type = EigenDirichletBC
    variable = disp_z
    boundary = bottom
  []
  [eigen_clamp_rot_x]
    type = EigenDirichletBC
    variable = rot_x
    boundary = bottom
  []
  [eigen_clamp_rot_y]
    type = EigenDirichletBC
    variable = rot_y
    boundary = bottom
  []
  [eigen_clamp_rot_z]
    type = EigenDirichletBC
    variable = rot_z
    boundary = bottom
  []
  [clamp_disp_x]
    type = DirichletBC
    variable = disp_x
    boundary = bottom
    value = 0.0
  []
  [clamp_disp_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []
  [clamp_disp_z]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0.0
  []
  [clamp_rot_x]
    type = DirichletBC
    variable = rot_x
    boundary = bottom
    value = 0.0
  []
  [clamp_rot_y]
    type = DirichletBC
    variable = rot_y
    boundary = bottom
    value = 0.0
  []
  [clamp_rot_z]
    type = DirichletBC
    variable = rot_z
    boundary = bottom
    value = 0.0
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
  [inertia_disp_x]
    type = DSGShellMassEigenKernel
    extra_vector_tags = 'eigen'
    component = 0
    variable = disp_x
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [inertia_disp_y]
    type = DSGShellMassEigenKernel
    extra_vector_tags = 'eigen'
    component = 1
    variable = disp_y
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [inertia_disp_z]
    type = DSGShellMassEigenKernel
    extra_vector_tags = 'eigen'
    component = 2
    variable = disp_z
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [inertia_rot_x]
    type = DSGShellMassEigenKernel
    extra_vector_tags = 'eigen'
    component = 3
    variable = rot_x
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [inertia_rot_y]
    type = DSGShellMassEigenKernel
    extra_vector_tags = 'eigen'
    component = 4
    variable = rot_y
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [inertia_rot_z]
    type = DSGShellMassEigenKernel
    extra_vector_tags = 'eigen'
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
    youngs_modulus = 2.1e6
    poissons_ratio = 0.3
    thickness = 0.1
  []
  [resultants]
    type = ComputeDSGShellResultants
  []
  [density]
    type = GenericConstantMaterial
    prop_names = 'density'
    prop_values = '1.0'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Eigenvalue
  solve_type = KRYLOVSCHUR
  which_eigen_pairs = SMALLEST_MAGNITUDE
  n_eigen_pairs = 3
  n_basis_vectors = 8
  petsc_options_iname = '-st_type -eps_target -st_pc_type'
  petsc_options_value = 'sinvert 0 lu'
  eigen_tol = 1e-8
[]

[Problem]
  type = EigenProblem
[]

[VectorPostprocessors]
  [omega_squared]
    type = Eigenvalues
    execute_on = TIMESTEP_END
  []
[]

[Outputs]
  csv = true
[]
