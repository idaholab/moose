# Part 3: Kernels id=kokkos_workshop_kernels

!---

## Anatomy of a Kokkos Kernel: Header

!listing framework/include/kokkos/kernels/KokkosDiffusion.h start=class

!---

## Anatomy of a Kokkos Kernel: Source

!listing framework/src/kokkos/kernels/KokkosDiffusion.K start=#include language=cpp

- `registerKokkosResidualObject()` replaces `registerMooseObject()` for kernels, nodal kernels,
  and boundary conditions; it also registers the device loops for your type
- Inside the class, `Kernel` refers to `Moose::Kokkos::Kernel`, so `Kernel::validParams()` is the
  Kokkos base class
- Host-only code (`validParams()`, constructor, `initialSetup()`, ...) is ordinary MOOSE code

!---

## Translating MOOSE to Kokkos-MOOSE

!style! fontsize=75%

| MOOSE | Kokkos-MOOSE |
| :- | :- |
| `class K : public Kernel` | `class K : public Moose::Kokkos::Kernel` |
| `registerMooseObject("App", K)` | `registerKokkosResidualObject("App", K)` |
| `virtual Real computeQpResidual() override` | `template <typename Derived> KOKKOS_FUNCTION Real computeQpResidual(i, qp, datum) const` |
| `_u[_qp]`, `_grad_u[_qp]` | `_u(datum, qp)`, `_grad_u(datum, qp)` |
| `_test[_i][_qp]`, `_grad_phi[_j][_qp]` | `_test(datum, i, qp)`, `_grad_phi(datum, j, qp)` |
| `_q_point[_qp]`, `_JxW[_qp]` | `datum.q_point(qp)`, `datum.JxW(qp)` (applied by the base class) |
| `_current_elem` | `datum.elem()` (contiguous element and subdomain IDs), `kokkosMesh()` |
| `_t`, `_dt`, `_t_step` | unchanged |
| `coupledValue("v")`, `coupledGradient("v")` | `kokkosCoupledValue("v")`, `kokkosCoupledGradient("v")` |
| `coupledDot("v")`, `coupledDotDu("v")` | `kokkosCoupledDot("v")`, `kokkosCoupledDotDu("v")` |
| `coupled("v")` | unchanged |
| `getMaterialProperty<Real>("k")` | `getKokkosMaterialProperty<Real>("k")` |
| `RealVectorValue`, `RealTensorValue` | `Moose::Kokkos::Real3`, `Moose::Kokkos::Real33` |

!style-end!

!---

## Hook Methods

All hooks are public, `const`, `KOKKOS_FUNCTION` templates on `Derived`. Only
`computeQpResidual()` is required; the base class skips Jacobian loops whose hook you did not
define.

```cpp
template <typename Derived>
KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i, const unsigned int qp,
                                       AssemblyDatum & datum) const;
template <typename Derived>
KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int i, const unsigned int j,
                                       const unsigned int qp, AssemblyDatum & datum) const;
template <typename Derived>
KOKKOS_FUNCTION Real computeQpOffDiagJacobian(const unsigned int i, const unsigned int j,
                                              const unsigned int jvar, const unsigned int qp,
                                              AssemblyDatum & datum) const;
template <typename Derived>
KOKKOS_FUNCTION Real computeQpOffDiagJacobianScalar(const unsigned int i, const unsigned int j,
                                                    const unsigned int jvar, const unsigned int qp,
                                                    AssemblyDatum & datum) const;
```

- Indices are arguments, not members, because every thread works on a different element
- `datum` carries the per-thread element context: `n_qps()`, `n_dofs()`, `JxW(qp)`,
  `q_point(qp)`, `elem()`, `subdomain()`

!---

## Optimized Kernels: `KernelValue` and `KernelGrad`

When the residual has the form $(\psi_i, f)$ or $(\nabla\psi_i, \vec{F})$, compute $f$ or
$\vec{F}$ once per quadrature point instead of once per test function. Prefer these bases
whenever the form allows.

| Base | Residual form | Hooks |
| :- | :- | :- |
| `Moose::Kokkos::KernelValue` | $(\psi_i, f)$ | `Real precomputeQpResidual(qp, datum)`, `Real precomputeQpJacobian(j, qp, datum)` |
| `Moose::Kokkos::KernelGrad` | $(\nabla\psi_i, \vec{F})$ | `Real3 precomputeQpResidual(qp, datum)`, `Real3 precomputeQpJacobian(j, qp, datum)` |

Both also accept `precomputeQpOffDiagJacobian(j, jvar, qp, datum)`.

!--

## `KernelGrad` Example: `KokkosHeatConduction`

!listing modules/heat_transfer/include/kokkos/kernels/KokkosHeatConduction.h start=class

!---

## Coupling and Off-Diagonal Jacobians

!listing framework/include/kokkos/kernels/KokkosCoupledTimeDerivative.h start=class

```cpp
KokkosCoupledTimeDerivative::KokkosCoupledTimeDerivative(const InputParameters & parameters)
  : KernelValue(parameters),
    _v_dot(kokkosCoupledDot("v")),
    _dv_dot(kokkosCoupledDotDu("v")),
    _v_var(coupled("v"))
{
}
```

!---

## Time Kernels and Automatic Differentiation

- Time derivative kernels derive from `Moose::Kokkos::TimeKernel` or `TimeKernelValue`; use
  `_u_dot(datum, qp)` and `_du_dot_du` (no `[_qp]` index)
- AD kernels derive from `Moose::Kokkos::ADKernel`, register with
  `registerKokkosADResidualObject()`, and return `Moose::Kokkos::ADReal`:

!listing framework/include/kokkos/kernels/KokkosADDiffusion.h start=class

- The Jacobian is assembled automatically; Jacobian hooks are not used
- AD is available for `ADKernel`, `ADIntegratedBC`, and `ADNodalBC`. Materials and functions do
  not propagate derivatives yet.

!---

## Extra Parallelism Within an Element

By default each element is handled by a small team of threads that split its local DOFs:

| Parameter | Default | Meaning |
| :- | :- | :- |
| `num_local_residual_threads` | 2 | Threads per element in residual assembly |
| `num_local_jacobian_threads` | 4 | Threads per element in Jacobian assembly |

- Available on kernels and integrated BCs, only for GPU builds (setting them with
  `--with-kokkos=cpu` is an error)
- The defaults were tuned for first-order Lagrange elements in 3D; higher-order elements can
  benefit from more threads. A divisor of the number of DOFs per element is a good choice.
- Your hooks do not change; the base class partitions the `i` (and `j`) loops

!---

# Exercise 3: Port a Nonlinear Diffusion Kernel id=kokkos_workshop_ex3

!---

## Exercise 3: Weak Form

Strong form on $\Omega$, with $k(u) = k_0 (1 + \beta u)$:

!equation
-\nabla \cdot \left(k(u) \nabla u\right) = 0

Multiply by a test function $\psi_i$ and integrate over $\Omega$:

!equation
-\int_\Omega \psi_i \nabla \cdot \left(k(u) \nabla u\right) \, d\Omega = 0

Integrate by parts using
$\nabla \cdot (\psi_i k \nabla u) = \nabla \psi_i \cdot k \nabla u + \psi_i \nabla \cdot (k \nabla u)$
and the divergence theorem:

!equation
\int_\Omega k(u) \nabla u \cdot \nabla \psi_i \, d\Omega
- \int_{\partial\Omega} \psi_i \, k(u) \nabla u \cdot \hat{n} \, d\Gamma = 0

- On Dirichlet boundaries $\psi_i = 0$, so the boundary term vanishes
- Elsewhere it is the flux; with no BC it is the natural condition $k \nabla u \cdot \hat{n} = 0$.
  A flux BC (e.g. `KokkosNeumannBC`) supplies this term.

Find $u_h$ satisfying the Dirichlet conditions such that, for every test function $\psi_i$,

!equation
R_i(u_h) = \left(k(u_h) \nabla u_h, \nabla \psi_i\right) = 0

!--

## Exercise 3: From Weak Form to Hooks

The base class performs the quadrature sum; the hook returns the integrand at one quadrature
point:

!equation
R_i = \sum_{qp} JxW_{qp} \,
\underbrace{k(u) \nabla u \cdot \nabla \psi_i}_{\texttt{computeQpResidual}}

Newton needs $J_{ij} = \partial R_i / \partial u_j$. With $u_h = \sum_j u_j \phi_j$:
$\partial u_h / \partial u_j = \phi_j$, $\partial \nabla u_h / \partial u_j = \nabla \phi_j$, and
$k'(u) = k_0 \beta$. By the product rule:

!equation
J_{ij} = \left(k(u_h) \nabla \phi_j + k_0 \beta \, \phi_j \nabla u_h, \nabla \psi_i\right)

| Term | MOOSE | Kokkos-MOOSE |
| :- | :- | :- |
| $u_h$, $\nabla u_h$ | `_u[_qp]`, `_grad_u[_qp]` | `_u(datum, qp)`, `_grad_u(datum, qp)` |
| $\nabla \psi_i$ | `_grad_test[_i][_qp]` | `_grad_test(datum, i, qp)` |
| $\phi_j$, $\nabla \phi_j$ | `_phi[_j][_qp]`, `_grad_phi[_j][_qp]` | `_phi(datum, j, qp)`, `_grad_phi(datum, j, qp)` |

The residual has the form $(\vec{F}, \nabla \psi_i)$ with $\vec{F} = k(u) \nabla u$, which is
why it fits `KernelGrad`: `precomputeQpResidual` returns $\vec{F}$ and `precomputeQpJacobian`
returns $\partial \vec{F} / \partial u_j = k \nabla \phi_j + k_0 \beta \phi_j \nabla u$.

!---

## Exercise 3: The Host Kernel

Port this host kernel for $-\nabla \cdot \left(k(u) \nabla u\right) = 0$ with
$k(u) = k_0 (1 + \beta u)$:

```cpp
Real
NonlinearDiffusion::computeQpResidual()
{
  const Real k = _k0 * (1 + _beta * _u[_qp]);
  return k * _grad_u[_qp] * _grad_test[_i][_qp];
}

Real
NonlinearDiffusion::computeQpJacobian()
{
  const Real k = _k0 * (1 + _beta * _u[_qp]);
  return (k * _grad_phi[_j][_qp] + _k0 * _beta * _phi[_j][_qp] * _grad_u[_qp]) *
         _grad_test[_i][_qp];
}
```

with parameters `k0` (default 1) and `beta` (default 0).

!---

## Exercise 3: Tasks

1. Write `include/kernels/KokkosNonlinearDiffusion.h` and `src/kernels/KokkosNonlinearDiffusion.K`
   deriving from `Moose::Kokkos::Kernel`; rebuild
1. Solve on $[0,1]^2$ with $u=0$ on `left`, $u=1$ on `right`, $k_0=1$, $\beta=2$, using
   `KokkosDirichletBC`. The exact solution is $u(x) = (\sqrt{1 + 8x} - 1)/2$, so
   $u(0.5) = (\sqrt{5} - 1)/2 \approx 0.618$. Check it with a `PointValue` postprocessor at
   $(0.5, 0.5)$.
1. Check the Jacobian: add `-snes_test_jacobian` to `petsc_options` on a coarse mesh
   (or write a `PetscJacobianTester` test)
1. Rename `computeQpJacobian` to `computeQpJacobain`, rebuild, and run: what happens to the
   solve, and what does the Jacobian test say? Then fix it
1. Rewrite the kernel on top of `Moose::Kokkos::KernelGrad`
1. Bonus (+GPU only+): time the solve on a $500 \times 500$ mesh with different
   `num_local_jacobian_threads`

!--

## Exercise 3: Solution Header

The complete, tested solutions are in `tutorials/kokkos_workshop/kokkos_training` (Exercise 3:
`KokkosNonlinearDiffusion`, `KokkosNonlinearDiffusionGrad`, `tests/ex3`).

!listing tutorials/kokkos_workshop/kokkos_training/include/kernels/KokkosNonlinearDiffusion.h start=#pragma link=False

!--

## Exercise 3: Solution Source and Input

!listing tutorials/kokkos_workshop/kokkos_training/src/kernels/KokkosNonlinearDiffusion.K start=#include language=cpp link=False

!listing tutorials/kokkos_workshop/kokkos_training/tests/ex3/ex3.i block=Kernels Postprocessors Executioner link=False

!--

## Exercise 3: Solution with `KernelGrad`

!listing tutorials/kokkos_workshop/kokkos_training/include/kernels/KokkosNonlinearDiffusionGrad.h start=class link=False

It is registered as a separate class, `KokkosNonlinearDiffusionGrad`, so both versions can be
run on the same input: `Kernels/diff/type=KokkosNonlinearDiffusionGrad`. The source file uses
`KernelGrad::validParams()` and `KernelGrad(parameters)`.

!--

## Exercise 3: Discussion

- With the misspelled Jacobian, the Jacobian loop is never launched and the matrix only holds the
  Dirichlet rows. The first linear solve fails (`DIVERGED_PC_FAILED`) and the run aborts;
  `-snes_test_jacobian` reports a relative difference of about 2.6. Nothing warns you at compile
  time.
- The `KernelGrad` version evaluates $k(u)\nabla u$ once per quadrature point instead of once per
  test function, which reduces both flops and memory reads
- Inputs and results are identical between the two versions; only the cost changes
