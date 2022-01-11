# EigenProblem

!syntax description /Problem/EigenProblem

`EigenProblem` is the primary eigenvalue system in the MOOSE framework.
It offers linear and nonlinear eigensolvers by leveraging [SLEPc](https://slepc.upv.es)
capabilities (mainly the EPS system). For linear problems,
the system supports one eigenvalue and multiple eigenvalues. For
nonlinear problems, we currently support solving for the minimum
eigenvalue. The development of nonlinear eigensolver was mainly motivated
by neutron transport criticality calculations, but the implementation
and algorithm are general and applicable to other applications.

A generalized (linear) eigenvalue problem can uniformly be rewritten as
\begin{equation}
Ax = \lambda Bx,
\end{equation}
$A$ and $B$ are matrices, $x$ is an eigenvector, and $\lambda$ is the eigenvalue.
Generally speaking, $A$ is a nonsingular matrix, and $B$ can be singular
(e.g., an eigenvalue problem with Dirichlet boundary conditions).
Many solvers in SLEPc can handle this linear eigenproblem.
We can choose these algorithms in the executioner block.
Here is an example with Jacobi Davidson:

!listing test/tests/problems/eigen_problem/eigensolvers/gipm.i
         block=Executioner
         link=False

Note that to use the linear solvers from SLEPc, we need to form accurate
$A$ and $B$ that will be utilized to represent the underlying physics.
If $A% and $B$ are incorrect, you will likely end with the
wrong eigenvalue and eigenvector. For that reason, if you
want to use linear eigenvalue solvers, you need to implement
Jacobian by hand or with AD in the corresponding kernels.
Details of all the linear eigensolvers can be found in
[SLEPc Users Manual](https://slepc.upv.es/documentation/slepc.pdf).

In a multiphysics environment, physics often is nonlinear. For instance,
 neutron transport with temperature feedback. In this case,
  $A$ and $B$ are not constant anymore; instead, they depend on
   the eigenvector directly or indirectly. To describe our nonlinear
    solver, we write a generalized nonlinear eigenvalue problem as,
\begin{equation}
A(x) = \lambda B(x),
\end{equation}
where $A(\cdot)$ and $B$ are nonlinear functions. In the EigenProblem
system, the kernels of $A(\cdot)$ are referred to as "noneigen,"
and the kernels of $B(\cdot)$ are referred to as "eigen."
To utilize Newton to solve this problem, we define $\lambda = \frac{1}{|B(x)|}$,
and rewrite the nonlinear eigen problem as
\begin{equation}
0=F(x) = A(x) - \lambda B(x).
\end{equation}
Newton can be applied to solve this problem. The only issue is that
the derivatives of $\lambda$ with respective to $x$ can be dense.
We use the Jacobian-free Newton method to overcome this difficulty,
where the Jacobian is computed via finite difference.
To handle various use cases, variants of Newton are provided
in this system. There is a current list:

- `PJFNK` - Preconditioned Jacobian-free Newton Krylov: The preconditioning matrix
  is formed using nonegein kernels. We do have an option to allow users to
  incorporate eigen kernels into the preconditioning matrix. Still, in
  general, we won't encourage users to do that since the preconditioning
  matrix might be singular when close to the solution.

!listing test/tests/problems/eigen_problem/eigensolvers/ne.i
          block=Executioner
          link=False

- `JFNK` - Jacobian-free Newton Krylov:  Users are responsible for
  providing a preconditioner. This allows users to build a customized
  preconditioner, such as a sweeper,
  that might be more efficient for their underlying problems.

!listing test/tests/problems/eigen_problem/preconditioners/ne_pbp.i
          block=Executioner Preconditioning
          link=False

- `PJFNKMO`- Matrix-only Preconditioned Jacobian-free Newton Krylov: Mathematically,
 this option is the same as PJFNK. The difference is the residual evaluation.
 PJFNK does `real` residual evaluations, including all finite element calculations.
 PJFNKMO forms residual vectors via  matrix-vector multiplications: $A (x) = Ax$
 and $B(x) = Bx$. This is useful when matrixes are constant. This option will be
 more efficient. If the matrices are not constant, please do not use this option;
 otherwise, simulation results will be wrong.

!listing test/tests/problems/eigen_problem/jfnk_mo/ne_array_mo.i
           block=Executioner
           link=False

- `Newton` - Newton method: Both Jacobian and preconditioning matrices are $A$.
 This option is added for the consistency between the nonlinear system and
 the nonlinear eigensystem. Since this option does not account for the derivative
 of eigenvalue with respective to the eigenvector, this option is not efficient.
 Users should not use it in general.

!syntax parameters /Problem/EigenProblem

!syntax inputs /Problem/EigenProblem

!syntax children /Problem/EigenProblem
