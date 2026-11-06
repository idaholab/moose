# Part 4: Polymorphism Without Virtual Functions id=kokkos_workshop_poly

!---

## MOOSE Is Built on Virtual Hooks

MOOSE systems follow the +template method pattern+: a base class owns the algorithm (element
loop, quadrature loop, assembly) and calls +hook methods+ that derived classes override.

```cpp
class Kernel // host MOOSE, simplified
{
public:
  void computeResidual() // template method: owns the loops
  {
    for (_i = 0; _i < _test.size(); _i++)
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        _local_re(_i) += _JxW[_qp] * _coord[_qp] * computeQpResidual(); // virtual call
  }
  virtual Real computeQpResidual() = 0; // hook method
};
```

On the device this breaks down:

- The vtable is filled by the constructor, and MOOSE objects are constructed on the +host+, so
  it holds host function pointers
- Device virtual calls need relocatable device code, which the MOOSE build does not enable; some
  backends do not support them at all
- Every virtual call is an indirect jump the compiler cannot inline, and GPU compilers depend on
  inlining for performance

!---

## Static Polymorphism: the CRTP

The Curiously Recurring Template Pattern resolves the hook at +compile time+:

!row!
!col! width=50%

Dynamic

```cpp
class Base
{
public:
  void compute() { implementation(); }
  virtual void implementation() { ... }
};

class Derived : public Base
{
public:
  void implementation() override { ... }
};
```

!col-end!

!col! width=50%

Static (CRTP)

```cpp
template <typename Derived>
class Base
{
public:
  void compute()
  {
    static_cast<Derived *>(this)->implementation();
  }
  void implementation() { ... }
};

class Derived : public Base<Derived>
{
public:
  void implementation() { ... } // hides Base's
};
```

!col-end!
!row-end!

| Dynamic | Static |
| :- | :- |
| `virtual` + `override` | redefine with the same signature (+hiding+) |
| pure virtual | no definition in the base; the compiler complains at the call |
| vtable lookup at run time | `static_cast` to the derived type at compile time, inlinable |
| hooks can be `protected` | hooks must be `public` (the base calls them from outside) |

!---

## Kokkos-MOOSE: Templated Hooks, Not Templated Classes

Making every base class a template (`Kernel<Derived>`) would force templates through every
intermediate class of an application. Kokkos-MOOSE keeps +ordinary base classes+ and makes the
+hooks+ member templates on the final type:

```cpp
class MyKernel : public Moose::Kokkos::Kernel // not a template
{
public:
  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i, const unsigned int qp,
                                         AssemblyDatum & datum) const;
};
```

The base class loops receive the final object and call its hook:

```cpp
template <typename Derived>
KOKKOS_FUNCTION void
Kernel::computeResidualInternal(const Derived & kernel, AssemblyDatum & datum) const
{
  ... local_re[i] += datum.JxW(qp) * kernel.template computeQpResidual<Derived>(i, qp, datum);
}
```

Inside any hook, `Derived` is the type you registered, so `static_cast<const Derived *>(this)`
is always safe.

!---

## How the Concrete Type Reaches the Device

The host only knows an object by its base pointer and its registered name. A +registry+ maps
(loop, name) to a dispatcher that knows the concrete type:

```text
Static initialization (registerKokkosResidualObject("App", MyKernel))
  DispatcherRegistry::addDispatcher<ResidualLoop, MyKernel>("MyKernel")

Host, every residual evaluation (Kernel::computeResidual)
  DispatcherRegistry::build<ResidualLoop>(this, type())  // type() == "MyKernel"
    -> Dispatcher<ResidualLoop, MyKernel>                // holds a MyKernel copy of *this
    -> Kokkos::parallel_for(policy, dispatcher)          // virtual call, on the host

Device, every thread
  Dispatcher::operator()(tid)
    -> kernel(ResidualLoop{}, tid, kernel)               // Kernel::operator()<MyKernel>
      -> kernel.computeResidualInternal(kernel, datum)
        -> kernel.computeQpResidual<MyKernel>(i, qp, datum)  // inlined
```

Type erasure (`DispatcherBase`, `virtual parallelFor()`) stays on the +host+. The device only
ever sees concrete types.

!---

## Optional Hooks Are Detected at Registration

There is no `override` keyword to check against, so the registration macro compares member
function pointers with the base class defaults:

```cpp
// From registerResidualObjectDispatchers<Object>() (simplified)
DispatcherRegistry::hasUserMethod<typename Object::JacobianLoop>(
    objectname,
    &Object::template computeQpJacobian<Object> != Object::template defaultJacobian<Object>());
```

```cpp
// Kernel::computeJacobian() on the host
if (DispatcherRegistry::hasUserMethod<JacobianLoop>(type()))
  ... launch the Jacobian loop ...
```

- An undefined optional hook costs nothing: its loop is never launched
- The flip side: a +misspelled+ hook is indistinguishable from an undefined one. The loop is
  skipped without a warning.
- The default hooks call `::Kokkos::abort()` with a message if they are ever reached

!---

## Writing Your Own Polymorphic Base Class

Use the same technique one level down: the base implements the framework hook and calls a
+sub-hook+ of `Derived`. `DirichletBCBase` does this (simplified from the AD-templated source):

```cpp
template <typename Derived>
KOKKOS_FUNCTION Real
DirichletBCBase::computeQpResidual(const unsigned int qp, AssemblyDatum & datum) const
{
  auto bc = static_cast<const Derived *>(this);
  return _u(datum, qp) - bc->computeValue(qp, datum);
}
```

!listing framework/include/kokkos/bcs/KokkosDirichletBC.h start=KOKKOS_FUNCTION end=protected:

- Register +only the final classes+; a base that is never registered is never instantiated, so
  it may call sub-hooks it does not define (the static equivalent of pure virtual)
- A base may define a default sub-hook that derived classes hide
- Sub-hooks must be `public` and `const`, but they need not be templates
- Call a hidden base version explicitly with `Base::template computeQpResidual<Derived>(i, qp,
  datum)`

!---

## Class-Template Bases for Code Reuse

Class templates are still useful when one implementation serves several bases. The integral
postprocessors share their reducer between element and side loops:

```cpp
template <typename Base> // Base = ElementPostprocessor or SidePostprocessor
class KokkosIntegralPostprocessor : public Base // simplified
{
  template <typename Derived>
  KOKKOS_FUNCTION void reduce(Datum & datum, Real * result) const
  {
    for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
      result[0] += datum.JxW(qp) *
                   static_cast<const Derived *>(this)->computeQpIntegral(qp, datum);
  }
};
```

- `KokkosIntegralVariablePostprocessor<Base>` defines `computeQpIntegral()` as $u$
- `KokkosElementL2Norm` hides it again with $u^2$: hiding works through any number of levels,
  because `Derived` is always the registered type

!---

## Adding Your Own Device Loop

Hooks are not the only extension point. An object can define extra loops with a +tag type+ and
an `operator()` overload. `DirichletBCBase` presets the solution on the boundary this way:

```cpp
struct PresetLoop {}; // tag type that names the loop

template <typename Derived>
KOKKOS_FUNCTION void operator()(PresetLoop, const ThreadID tid, const Derived & bc) const;

using Base::operator(); // keep the base class loops visible
```

```cpp
// Host launch
Policy policy(0, numKokkosBoundaryNodes());
auto dispatcher = DispatcherRegistry::build<PresetLoop>(this, this->type());
dispatcher->parallelFor(policy);
```

```cpp
// Registration: the residual loops plus the extra one
registerKokkosResidualObject(app, classname);
registerKokkosAdditionalOperation(classname, PresetLoop)
```

The same pattern lets an object override a whole loop body, e.g. `KokkosCopyValueAux` redefines
`computeElementInternal()` and `computeNodeInternal()` instead of `computeValue()`.

!---

## When You Really Need Run-Time Polymorphism

Sometimes the concrete type is not known where the object is used, e.g. a kernel that accepts
+any+ function. Kokkos-MOOSE handles this with a +wrapper+ (`Moose::Kokkos::Function`):

- For each registered function type, a host wrapper constructs a device object
  `FunctionWrapperDevice<Object>` +on the device+, so its vtable holds device pointers
- Its virtual shims forward to the concrete type's inlined methods: one virtual call, then
  static code

```cpp
template <typename Object>
class FunctionWrapperDevice : public FunctionWrapperDeviceBase
{
  KOKKOS_FUNCTION Real value(Real t, Real3 p) const KOKKOS_OVERRIDE
  {
    return _function->value(t, p);
  }
  ...
  Object * _function = nullptr;
};
```

This needs relocatable device code, so on GPU builds it is rejected today; it works with
`--with-kokkos=cpu`. Until then, retrieve functions and user objects by +concrete type+.

!---

# Exercise 4: A Polymorphic Boundary Condition Family id=kokkos_workshop_ex4

!---

## Exercise 4: Tasks

Convective flux $-k \nabla u \cdot \hat{n} = h(u) (u - u_\infty)$, i.e. residual
$(\psi_i, h(u)(u - u_\infty))$ and Jacobian $(\psi_i, [h + h'(u)(u - u_\infty)] \phi_j)$.

1. Write `KokkosConvectiveBCBase` on `Moose::Kokkos::IntegratedBCValue`. It implements both
   precompute hooks using two sub-hooks of `Derived`: `computeCoefficient(qp, datum)` (required)
   and `computeCoefficientDerivative(qp, datum)` (default 0 in the base). Do +not+ register it.
1. Write two `final` classes and register them:

   - `KokkosConstantConvectiveBC`: $h$ from a +controllable+ parameter `h`
   - `KokkosLinearConvectiveBC`: $h = h_0 (1 + \gamma u)$, so $h' = h_0 \gamma$

1. Apply each on `top` of the Exercise 3 problem and run a Jacobian test
1. Misspell `computeCoefficientDerivative` in `KokkosLinearConvectiveBC`. Does it compile? What
   does the Jacobian test say?
1. Bonus: ramp `h` with a `RealFunctionControl` (`parameter = 'BCs/top/h'`) and confirm the BC
   sees each new value without extra code

!--

## Exercise 4: Solution, Base Class

!listing tutorials/kokkos_workshop/kokkos_training/include/bcs/KokkosConvectiveBCBase.h start=#pragma link=False

!listing tutorials/kokkos_workshop/kokkos_training/src/bcs/KokkosConvectiveBCBase.K start=#include language=cpp link=False

The source file has +no+ registration macro.

!--

## Exercise 4: Solution, Derived Classes

!listing tutorials/kokkos_workshop/kokkos_training/include/bcs/KokkosConstantConvectiveBC.h start=class link=False

!listing tutorials/kokkos_workshop/kokkos_training/include/bcs/KokkosLinearConvectiveBC.h start=class link=False

!listing tutorials/kokkos_workshop/kokkos_training/src/bcs/KokkosConstantConvectiveBC.K start=registerKokkos language=cpp link=False

Inputs: `tests/ex4/ex4_constant.i`, `ex4_linear.i`, and `ex4_control.i` (the `RealFunctionControl`
bonus).

!--

## Exercise 4: Discussion

- The misspelled `computeCoefficientDerivative` compiles: the call through `Derived` finds the
  base default, which returns 0. The Jacobian test reports the missing $h'(u)(u - u_\infty)$
  term. This is the static analogue of forgetting `override`, and only tests catch it.
- Removing `computeCoefficient` from a derived class is a +compile error+, because the base has no
  default: the static equivalent of a pure virtual function
- Both BCs share one implementation of the residual and Jacobian, and every call is resolved and
  inlined at compile time. There is no run-time cost to the abstraction.
- The controllable `h` works because `Scalar<const Real>` re-reads the parameter in its copy
  constructor, which runs at every launch
