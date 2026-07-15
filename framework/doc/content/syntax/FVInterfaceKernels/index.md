# FVInterfaceKernels System

For an overview of MOOSE FV please see [/fv_design.md].

`FVInterfaceKernels` communicate data across interfaces between subdomains. The user defines the
orientation of an interface with `subdomain1` and `subdomain2`. `variable1` must be defined on
`subdomain1`, and `variable2` must be defined on `subdomain2`. If `variable2` is omitted,
`variable1` is used on both sides.

## Developer contract

Derived `FVInterfaceKernel` objects operate entirely in the user-defined side-1/side-2
orientation. `computeQpResidual()` returns the residual contribution \(r\) for `variable1` on
`subdomain1`. The base class multiplies \(r\) by the face area and coordinate factor, adds it to
`variable1`, and adds \(-r\) to `variable2` when both variables belong to the same nonlinear
system. A derived object does not need to determine which user-defined side is represented by
`FaceInfo::elem`.

The protected side-oriented accessors are:

- `normal()`, which points from side 1 toward side 2;
- `elem1()` and `elem2()`, which return the elements on sides 1 and 2;
- `centroid1()` and `centroid2()`, which return their centroids;
- `elemArg1()` and `elemArg2()`, which create element functor arguments; and
- `faceArg1()` and `faceArg2()`, which create explicitly one-sided face functor arguments.

`interpolateValue(method, value1, value2)` interpolates a value from each user-defined side while
accounting for the geometric orientation of the current `FaceInfo`. For example, a diffusion
interface can evaluate and interpolate its coefficients without any geometric orientation branch:

```
ADReal
FVFooInterface::computeQpResidual()
{
  const auto state = determineState();
  const auto coefficient = interpolateValue(
      _interp_method, _coefficient1(elemArg1(), state), _coefficient2(elemArg2(), state));

  Point one_over_gradient_support = centroid1() - centroid2();
  one_over_gradient_support /= one_over_gradient_support.norm_sq();
  const auto gradient = (var1().getElemValue(&elem1(), state) -
                         var2().getElemValue(&elem2(), state)) *
                        one_over_gradient_support;

  return -coefficient * normal() * gradient;
}
```

Use `faceArg1()` or `faceArg2()` when a functor must be evaluated from one particular side rather
than interpolated across the interface. This is important for discontinuous or block-restricted
functors.

!alert! note
When using an `FVInterfaceKernel` to connect variables that belong to different nonlinear systems,
create two kernels with flipped variable, subdomain, and material-property parameters. An
interface kernel contributes only to the system containing `variable1`. For an example, see:

!listing /test/tests/fviks/diffusion/multisystem.i

!alert-end!
