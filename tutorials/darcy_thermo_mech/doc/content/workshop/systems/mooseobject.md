# MooseObject

All user-facing objects in MOOSE derive from `MooseObject`, this allows for a common structure
for all applications and is the basis for the modular design of MOOSE.

!---

## Basic Header: CustomObject.h

```C++
#pragma once

#include "BaseObject.h"

class CustomObject : public BaseObject
{
public:
  static InputParameters validParams();

  CustomObject(const InputParameters & parameters);

protected:

  virtual Real doSomething() override;

  const Real & _scale;
};
```

!---

## Basic Source: CustomObject.C

```C++
#include "CustomObject.h"

registerMooseObject("CustomApp", CustomObject);

InputParameters
CustomObject::validParams()
{
  InputParameters params = BaseObject::validParams();
  params.addClassDescription("The CustomObject does something with a scale parameter.");
  params.addParam<Real>("scale", 1, "A scale factor for use when doing something.");
  return params;
}

CustomObject::CustomObject(const InputParameters & parameters) :
    BaseObject(parameters),
    _scale(getParam<Real>("scale"))
{
}

double
CustomObject::doSomething()
{
  // Do some sort of import calculation here that needs a scale factor
  return _scale;
}
```
