<!-- MOOSE Documentation Stub: Remove this when content is added. -->

# ICs System

- Allows for creation of arbitrary initial conditions.

  - Can be spatially dependent.

- Allows for selection of initial condition from input file.
- If using ExodusII mesh you can read the value of a variable from a previous simulation.
- Allows initial conditions to be coupled together.

  - Automatic dependency resolution.

## Base class

- `value(const Point & p)`

  - Returns the value the variable should have at point `p`.
  - All `InitialCondition` classes +MUST+ override this !

- `gradient(const Point & p)`

  - Returns the gradient the variable should have at point `p`.
  - This is optional. If you need this... you will know it!

## Steps for defining a new IC

- In your application create a new `.C` and `.h` for your new `InitialCondition` class under `src/ics` and `include/ics`.

  - Don't forget to them!

- Create a new class inside these files that inherits from `InitialCondition`

  - Make sure to override `value()`!
  - Note that `value()` is a pubic method.

- Inside your `Application.C`, `#include` your new `.h` file and register your new IC with the `MooseFactory`.
- Use it in an input file.

## ExampleIC.h/.C

```cpp
    #ifndef EXAMPLEIC_H
    #define EXAMPLEIC_H

    #include "InitialCondition.h"

    class ExampleIC;
    template<>
    # InputParameters validParams<ExampleIC>();

    class ExampleIC : public InitialCondition
    {
    public:
    # ExampleIC(const std::string | name,
              InputParameters parameters);

    virtual Real value(const Point | p);

    private:
      Real _coefficient;
    };

    #endif //EXAMPLEIC_H
```

```cpp
    #include "ExampleIC.h"

    template<>
    # InputParameters validParams<ExampleIC>()
    {
      InputParameters params = validParams<InitialCondition>();
      params.addRequiredParam<Real>("coefficient", "A coef");
      return params;
    }

    # ExampleIC::ExampleIC(const std::string | name,
                         InputParameters parameters):
      InitialCondition(name, parameters),
      _coefficient(getParam<Real>("coefficient"))
    {}

    # Real
    # ExampleIC::value(const Point | p)
    {
      // 2.0 * c * |x|
      return 2.0*_coefficient*std::abs(p(0));
    }
```

## Using ExampleIC

Now register it:

```cpp
    #include "ExampleIC.h"

    ...

    registerInitialCondition(ExampleIC);
```

And use it in an input file:

```puppet
...
[ICs]
  [./mat_1]
    type = ExampleIC
    variable = u
    coefficient = 2.0
    block = 1
  [../]

  [./mat_2]
    type = ExampleIC
    variable = u
    coefficient = 10.0
    block = 2
  [../]
...
```

## Initial Condition Shortcut Syntax

Constant Initial Conditions

```puppet
...
[ICs]
  [./mat_1]
    type = ExampleIC
    variable = u
    coefficient = 2.0
    block = 1
  [../]

  [./mat_2]
    type = ExampleIC
    variable = u
    coefficient = 10.0
    block = 2
  [../]
...
```

"Restart" from an existing solution

```puppet
...

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
    # For reading a solution
    # from an ExodusII file
    initial_from_file_var = diffused}
    initial_from_file_timestep = 2
  [../]

...
```

## Example 7

Look at [Example 7](ex07_ics.md)

## Further ICs documentation

!syntax list /ICs objects=True actions=False subsystems=False

!syntax list /ICs objects=False actions=False subsystems=True

!syntax list /ICs objects=False actions=True subsystems=False

