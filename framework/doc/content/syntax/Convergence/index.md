# Convergence System

The Convergence system allows users to customize the stopping criteria for the
iteration in various solves:

- Nonlinear system solves
- Linear system solves (not yet implemented)
- Steady-state detection in [Transient.md] (not yet implemented)
- Fixed point solves with [MultiApps](syntax/MultiApps/index.md) (not yet implemented)
- Fixed point solves with multiple systems (not yet implemented)

Instead of supplying convergence-related parameters directly to the executioner,
the user creates `Convergence` objects whose names are then supplied to the
executioner, e.g.,

```
[Convergence]
  [my_convergence1]
    type = MyCustomConvergenceClass
    # some convergence parameters, like tolerances
  []
[]

[Executioner]
  type = Steady
  nonlinear_convergence = my_convergence1
[]
```

Currently only the nonlinear solve convergence is supported, but others are planned
for the near future. If the `nonlinear_convergence` parameter is not specified,
then the default `Convergence` associated with the problem is created internally.

!syntax list /Convergence objects=True actions=False subsystems=False

!syntax list /Convergence objects=False actions=False subsystems=True

!syntax list /Convergence objects=False actions=True subsystems=False

## Convergence Criteria Design Considerations

Here we provide some considerations to make in designing convergence criteria
and choosing appropriate parameter values.
Consider a system of algebraic system of equations

!equation
\mathbf{r}(\mathbf{u}) = \mathbf{0} \,,

where $\mathbf{u}$ is the unknown solution vector, and $\mathbf{r}$ is the residual
function. To solve this system using an iterative method, we must decide on
criteria to stop the iteration.
In general, iteration for a solve should halt when the approximate solution $\tilde{\mathbf{u}}$
has reached a satisfactory level of error $\mathbf{e} \equiv \tilde{\mathbf{u}} - \mathbf{u}$,
using a condition such as

!equation id=error_criteria
\|\mathbf{e}\| \leq \tau_u \,,

where $\|\cdot\|$ denotes some norm, and $\tau_u$ denotes some tolerance.
Unfortunately, since we do not know $\mathbf{u}$, the error $\mathbf{e}$ is
also unknown and thus may not be computed directly. Thus some approximation of
the condition [!eqref](error_criteria) must be made. This may entail some
approximation of the error $\mathbf{e}$ or some criteria which implies the
desired criteria. For example, a very common approach is to use a residual
criteria such as

!equation
\|\mathbf{r}\| \leq \tau_{r,\text{abs}} \,.

While it is true that $\|\mathbf{r}\| = 0$ implies $\|\mathbf{e}\| = 0$, a
zero-tolerance is impractical, and the translation between the tolerance
$\tau_u$ to the tolerance is $\tau_r$ is difficult. The "acceptable" absolute
residual tolerance is tricky to determine and is highly dependent on the
equations being solved. To attempt to circumvent this issue, relative residual
criteria have been used, dividing the residual norm by another value in an
attempt to normalize it. A common approach that has been used is to use the
initial residual vector $\mathbf{r}_0$ to normalize:

!equation
\frac{\|\mathbf{r}\|}{\|\mathbf{r}_0\|} \leq \tau_{r,\text{rel}} \,,

where $\tau_{r,\text{rel}}$ is the relative residual tolerance. The disadvantage
with this particular choice is that this is highly dependent on how good the
initial guess is: if the initial guess is very good, it will be nearly impossible
to converge to the tolerance, and if the initial guess is very bad, it will be
too easy to converge to the tolerance, resulting in an erroneous solution.

Some other considerations are the following:

- Consider round-off error: if error ever reaches values around round-off error,
  the solve should definitely be considered converged, as iterating further
  provides no benefit.
- Consider the other sources of error in the model that produced the system of
  algebraic equations that you're solving. For example, if solving a system of
  partial differential equations, consider the model error and the discretization
  error; it is not beneficial to require algebraic error less than the other
  sources of error.
- Since each convergence criteria typically has some weak point where they break
  down, it is usually advisable to use a combination of criteria.

For more information on convergence criteria, see [!cite](rao2018stopping) for
example.

!alert tip title=Create your own convergence action
The `Convergence` system provides a lot of flexibility by providing several
pieces that can be combined together to create a desired set of convergence
criteria. Since this may involve a large number of objects (including objects
from other systems), it may be beneficial to create an [Action](/Action.md)
to create more compact and convenient syntax for your application.

## Implementing a New Convergence Class

`Convergence` objects are responsible for overriding the virtual method

```
MooseConvergenceStatus checkConvergence(unsigned int iter)
```

The returned type `MooseConvergenceStatus` is one of the following values:

- `CONVERGED`: The system has converged.
- `DIVERGED`: The system has diverged.
- `ITERATING`: The system has neither converged nor diverged and thus will
  continue to iterate.

