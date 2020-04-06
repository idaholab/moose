# MooseObject

All user-facing objects in MOOSE derive from `MooseObject`, this allows for a common structure
for all applications and is the basis for the modular design of MOOSE.

There are two basic forms of a MooseObject, a basic form and a template form for [!ac](AD).

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

!---

## AD Header: ADCustomObject.h

```cpp
#pragma once

#include "ADBaseObject.h"

class ADCustomObject : public ADBaseObject
{
public:
  static InputParameters validParams();

  ADCustomObject(const InputParameters & parameters);

protected:
  virtual ADReal doSomething() override;

  const Real & _scale;
};
```

!---

## AD Source: ADCustomObject.C

```cpp
#include "ADCustomObject.h"

registerMooseObject("MooseApp", ADCustomObject);

InputParameters
ADCustomObject::validParams()
{
    InputParameters params = ADCustomObject::validParams();
    params.addParam<Real>("scale", 1, "A scale factor for use when doing something.");
    params.addClassDescription("The ADCustomObject does something with a scale parameter.");
    params;
)

ADCustomObject::ADCustomObject(const InputParameters & parameters)
  : ADBaseObject(parameters),
    _scale(getParam<Real>("scale"))

{
}

ADReal
ADCustomObject::doSomething()
{
  return _scale;
}
```
