# DerivativeMaterialInterface

In model development, in particular using the finite element method, it is often
convenient to encapsulate expression values *together with their derivatives*.
The derivatives are often needed to compute Jacobian matrix entries.

In MOOSE we use material properties to provide values for expressions that can
be directly computed from variables in the solve. To store the derivatives of
these material properties with respect to the variables they depend on we also
use material properties. We use a standardized naming scheme to name the
material property derivatives. The derivative of a material property `F` with
respect to a variable `c` would be named `dF/dc`.

To enforce this naming scheme we provide the `DerivativeMaterialInterface`,
a *veneer* template that provides methods to *declare* and *get* material
properties that are derivatives of other material properties.

The `DerivativeMaterialInterface` is utilized by inheriting from it and
supplying the original parent class as a template argument.

## Examples

The  can be used from Materials (to declare and get material property
derivatives) or from Kernels (to get material property derivatives).

## Use in a Material class

### Header

```c++
#include "DerivativeMaterialInterface.h"
#include "Material.h"

//...

// we template the interface on 'Material'
class MyMaterial : public DerivativeMaterialInterface<Material>
{

//...

private:
  MaterialProperty<Real> & F;
  MaterialProperty<Real> & dFdc;
  MaterialProperty<Real> & d2Fdc2;
};
```

Note that it is possible to template the interface on arbitrary classes,
including classes derived from `Material` or `Kernel`.

### Implementation

```c++
#include "MyMaterial.h"

//...

MyMaterial::MyMaterial(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),
    // get the c variable value, number, and name
    _c(coupledValue("c")),
    _c_var(coupled("c")),
    _c_name(getVar("c", 0)->name()),
    // declare material property and first and second derivative w.r.t. c
    F(declareProperty<Real>("F")),
    dFdc(declarePropertyDerivative<Real>("F", _c_name))
    d2Fdc2(declarePropertyDerivative<Real>("F", _c_name, _c_name))
{
}
```

Material property derivatives do *not* need to be declared or computed in the
same Material as their corresponding undifferentiated properties (though
oftentimes it is a natural choice that provides encapsulation).

## Use in a Kernel class

### Header

```c++
#include "DerivativeMaterialInterface.h"
#include "Kernel.h"

//...

// we template the interface on 'Kernel'
class MyKernel : public DerivativeMaterialInterface<Kernel>
{

//...

protected:
  const MaterialProperty<Real> & F;
  const MaterialProperty<Real> & dFdc;
};
```

Note that material property derivatives that are fetched (and *not* declared) in
a class need to be stored in `const` references. It is recommended to use
constant references for regular material properties as well, if they are not
written to.

### Implementation

```c++
#include "MyKernel.h"

//...

MyKernel::MyKernel(const InputParameters & parameters) :
    DerivativeMaterialInterface<Kernel>(parameters),
    // get the c variable value, number, and name
    _c(coupledValue("c")),
    _c_var(coupled("c")),
    _c_name(getVar("c", 0)->name()),
    // fetch material property and first derivative w.r.t. c
    F(getMaterialProperty<Real>("F")),
    dFdc(getMaterialPropertyDerivative<Real>("F", _c_name))
{
}
```

Here we may also request other derivatives, such as

```
  // get the eta variable value, number, and name
  _eta(coupledValue("eta")),
  _eta_var(coupled("eta")),
  _eta_name(getVar("eta", 0)->name()),
  // fetch material property and first derivative w.r.t. eta
  dFdeta(getMaterialPropertyDerivative<Real>("F", _eta_name))
```

The *eta* derivative may not necessarily be declared anywhere in the simulation.
The interface will return a default value of zero in that case (see below).

# Default values

When requesting non-existing material property derivatives using any of the
`getMaterialPropertyDerivative` methods, a *zero* object will be returned.
For `Real` types this will be `0` for vectors and tensors those will be
objects with zeroes in all entries.

Knowing this, kernels utilizing the derivatives should always implement the most
complete expressions including all possible derivatives, even though they might
not be provided in every simulation.
