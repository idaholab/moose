# [UserObject System](syntax/UserObjects/index.md)

A system for defining an arbitrary interface between MOOSE objects.

!---

The UserObject system provides data and calculation results to other MOOSE objects.

- Postprocessors are UserObjects that compute a single scalar value.
- VectorPostprocessors are UserObjects that compute vectors of data.
- UserObjects define their own interface, which other MOOSE objects can call to retrieve data.

!---

## Execution

UserObjects are computed at specified "times" by the execute_on option in the input file:

`execute_on = 'initial timestep_end'`\\
`execute_on = linear`\\
`execute_on = nonlinear`\\
`execute_on = 'timestep_begin final failed'`

They can be restricted to specific blocks, sidesets, and nodesets

!---

## UserObject Types

There are various types of UserObjects:

- +ElementUserObject+: executes on elements
- +NodalUserObject+: executes on nodes
- +SideUserObject+: executes on sides on boundaries
- +InternalSideUserObject+: executes on internal sides
- +InterfaceUserObject+: executes on sides on interfaces
- +GeneralUserObject+: executes once

!---

## UserObject Anatomy

`virtual void initialize();`\\
Called once before beginning the `UserObject` calculation.

`virtual void execute();`\\
Called once on each geometric entity (element, node, etc.) or once per calculation for a
`GeneralUserObject`.

!---

`virtual void threadJoin(const UserObject & uo);`\\
During threaded execution this function is used to "join" together calculations generated on
different threads.

- Cast `uo` to a `const` reference of the specific UserObject type,
  then extract data and aggregate it to the data in "this" object.
- This is not required for a `GeneralUserObject` because it is +not+ threaded.

`virtual void finalize();`\\
The last function called after all calculations have been completed.

- Take data from all calculations performed in `execute()` and perform an operation to get the final
  value(s)
- Perform parallel communication where necessary to ensure all processors compute the same value(s)

!---

## User Defined Interface

A `UserObject` defines its own interface by defining `const` functions.

For example, if a `UserObject` is computing the average value of a variable on every block in the
mesh, it might provide a function like:

```cpp
Real averageValue(SubdomainID block) const;
```

Another MooseObject needing this `UserObject` would then call `averageValue()` to get the result of
the calculation.

!---

## Using a UserObject

Any MOOSE object can retrieve a `UserObject` in a manner similar to retrieving a `Function`.

Generally, it is a good idea to take the name of the `UserObject` from the input file:

```cpp
InputParameters
BlockAverageDiffusionMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<UserObjectName>("block_average_userobject", "Computes the ...");
  return params;
}
```

!---

A `UserObject` comes through as a `const` reference of the `UserObject` type. So, in your object:

```cpp
const BlockAverageValue & _block_average_value;
```

The reference is set in the initialization list of your object by calling the templated
`getUserObject()` method:

```cpp
BlockAverageDiffusionMaterial::BlockAverageDiffusionMaterial(const InputParameters & parameters) :
    Material(parameters),
    _block_average_value(getUserObject<BlockAverageValue>("block_average_userobject"))
{}
```

!---

Use the reference by calling some of the interface functions defined by the `UserObject`:

```cpp
_diffusivity[_qp] = 0.5 * _block_average_value.averageValue(_current_elem->subdomain_id());
```
