# Part 5: Materials id=kokkos_workshop_materials

!---

## Anatomy of a Kokkos Material

!listing modules/heat_transfer/include/kokkos/materials/KokkosHeatConductionMaterial.h start=class

!listing modules/heat_transfer/src/kokkos/materials/KokkosHeatConductionMaterial.K start=KokkosHeatConductionMaterial::KokkosHeatConductionMaterial( language=cpp

!---

## Producing and Consuming Properties

| MOOSE | Kokkos-MOOSE |
| :- | :- |
| `registerMooseObject("App", M)` | `registerKokkosMaterial("App", M)` |
| `virtual void computeQpProperties() override` | `template <typename Derived> KOKKOS_FUNCTION void computeQpProperties(qp, datum) const` |
| `MaterialProperty<T> & _p` | `Moose::Kokkos::MaterialProperty<T, dim> _p` (by value) |
| `declareProperty<T>("p")` | `declareKokkosProperty<T, dim>("p", dims)` |
| `getMaterialProperty<T>("p")` | `getKokkosMaterialProperty<T, dim>("p")` |
| `getMaterialPropertyOld<T>("p")` | `getKokkosMaterialPropertyOld<T, dim>("p")` (or `state = 1`) |
| `hasMaterialProperty<T>("p")` | `hasKokkosMaterialProperty<T, dim>("p")` |
| `_p[_qp] = value` | `_p(datum, qp) = value` |

- Materials receive a `Datum` (no test/trial functions), not an `AssemblyDatum`
- Coupled variables work as in kernels: `kokkosCoupledValue()`, `kokkosCoupledGradient()`, ...

!---

## Property Types and Dimensions

Property types must be +trivial+: no user-defined constructors (including default member
initializers), no virtual functions, no heap-allocated members.

Instead of `std::vector` properties, use multi-dimensional properties. The dimension is a template
argument; the sizes are given at declaration and may differ between subdomains:

```cpp
// Declaration (constructor)
_stress = declareKokkosProperty<Real, 2>("stress", {3, 3});

// Evaluation (computeQpProperties)
auto stress = _stress(datum, qp); // thin accessor for this quadrature point; keep it local
for (unsigned int i = 0; i < 3; ++i)
  for (unsigned int j = 0; j < 3; ++j)
    stress(i, j) = ...;
```

For scalar properties the accessor converts to and assigns from `T` directly:
`_k(datum, qp) = 2 * _k_ref(datum, qp);`

!---

## Stateful Properties

Request the old or older state; define `initQpStatefulProperties()` to set the initial value:

```cpp
template <typename Derived>
KOKKOS_FUNCTION void initQpStatefulProperties(const unsigned int qp, Datum & datum) const;
```

!listing test/include/kokkos/materials/KokkosStatefulTest.h start=template end=private:

!---

## Optional and On-Demand Properties

There is no separate "optional property" type. An uninitialized `MaterialProperty` evaluates to
`false`:

```cpp
// Consumer constructor (KokkosHeatConduction)
_thermal_conductivity = getKokkosMaterialProperty<Real>("thermal_conductivity");
if (hasKokkosMaterialProperty<Real>("d_thermal_conductivity_dT"))
  _d_thermal_conductivity_dT = getKokkosMaterialProperty<Real>("d_thermal_conductivity_dT");

// Consumer hook
if (_d_thermal_conductivity_dT)
  jac += _d_thermal_conductivity_dT(datum, qp) * _phi(datum, j, qp) * _grad_u(datum, qp);
```

An +on-demand+ property is only allocated if some other object consumes it. The producer must
check before writing:

```cpp
_dk_dT = declareKokkosOnDemandProperty<Real>("dk_dT"); // constructor
if (_dk_dT)                                            // computeQpProperties
  _dk_dT(datum, qp) = ...;
```

!---

## Memory: Every Property Is Stored Everywhere

Host MOOSE evaluates properties element by element and only stores stateful ones. On the GPU all
elements are evaluated at once, so +every property is stored at every quadrature point+.

Ways to reduce memory and work:

- `constant_on = ELEMENT` or `SUBDOMAIN`: store one value per element or subdomain
- On-demand properties for outputs that are often unused (derivatives, diagnostics)
- Face copies of a material (`_bnd == true`) are separate objects: do not declare or compute
  properties there that no boundary object uses

!---

## Material Limitations

- No AD material properties yet
- Functor, interface, and discrete materials are not supported
- Material property output (`outputs = exodus` on a material) is not supported; use
  `KokkosMaterialRealAux` to copy a property into an auxiliary variable
- Kokkos properties are consumed through the Kokkos API (`getKokkosMaterialProperty`), i.e. by
  Kokkos objects

!---

# Exercise 5: Coupled, On-Demand, and Stateful Materials id=kokkos_workshop_ex5

!---

## Exercise 5: Tasks

1. Write a material `KokkosLinearConductivity` that couples a variable `temperature` and produces

   - `conductivity` $= k_0 (1 + \beta T)$
   - +on-demand+ `conductivity_derivative` $= k_0 \beta$

1. Write `KokkosMatNonlinearDiffusion` (on `KernelGrad`) that consumes `conductivity` and,
   +if it exists+, `conductivity_derivative` for the Jacobian
1. Re-run the Exercise 3 problem with the material and the new kernel: $u(0.5) \approx 0.618$
1. Point the kernel's `conductivity_derivative` parameter at a property name that does not exist.
   What happens to the nonlinear iterations, and why is the derivative no longer computed?
1. +Stateful+: write `KokkosRunningMax`, which stores the maximum of a coupled variable over time
   in `running_max`. Visualize it with `KokkosMaterialRealAux` on the transient problem on the
   next slide.

!--

## Exercise 5: Transient Problem for the Running Maximum

!style! fontsize=80%

!listing tutorials/kokkos_workshop/kokkos_training/tests/ex5/ex5_running_max.i block=Kernels Materials AuxVariables AuxKernels Postprocessors link=False

!style-end!

Zero `KokkosDirichletBC`s on `left` and `right`. $u$ rises and falls with the source; `u_max`
keeps the peak.

!--

## Exercise 5: Solution, Material

!listing tutorials/kokkos_workshop/kokkos_training/include/materials/KokkosLinearConductivity.h start=class link=False

!listing tutorials/kokkos_workshop/kokkos_training/src/materials/KokkosLinearConductivity.K start=registerKokkos language=cpp link=False

!--

## Exercise 5: Solution, Kernel

!listing tutorials/kokkos_workshop/kokkos_training/include/kernels/KokkosMatNonlinearDiffusion.h start=class link=False

!listing tutorials/kokkos_workshop/kokkos_training/src/kernels/KokkosMatNonlinearDiffusion.K start=KokkosMatNonlinearDiffusion::KokkosMatNonlinearDiffusion( language=cpp link=False

!--

## Exercise 5: Solution, Stateful Material

!listing tutorials/kokkos_workshop/kokkos_training/include/materials/KokkosRunningMax.h start=class link=False

!listing tutorials/kokkos_workshop/kokkos_training/src/materials/KokkosRunningMax.K start=KokkosRunningMax::KokkosRunningMax( language=cpp link=False

!--

## Exercise 5: Discussion

- When the kernel does not find `conductivity_derivative`, it never requests it, so the on-demand
  property is never allocated and the material skips it. The Jacobian loses the
  $\partial k / \partial u$ term and Newton converges more slowly (but to the same answer).
- The derivative is only correct when the material's `temperature` is the kernel's `variable`;
  host MOOSE has the same caveat
- Requesting the old state of `running_max` makes the property stateful: Kokkos-MOOSE shifts
  current to old at every new time step, as host MOOSE does
