# MooseEnum

`MooseEnum` and `MultiMooseEnum` are utility types used throughout MOOSE for parameters with a fixed
set of valid string values. They provide validation, optional defaults, and a convenient way to map
validated input values back to C++ enums in implementation code.

## `MooseEnum`

`MooseEnum` stores one selected value from a list of valid options. The constructor takes a
space-delimited list of allowed names and may also take a default value:

```cpp
MooseEnum point_not_found_behavior("ERROR WARNING IGNORE", "IGNORE");
params.addParam<MooseEnum>("point_not_found_behavior",
                           point_not_found_behavior,
                           "Behavior to use when a point cannot be located.");
```

Values can optionally include explicit integer IDs using `=`:

```cpp
MooseEnum constant_on("NONE=0 ELEMENT=1 SUBDOMAIN=2", "NONE");
```

When a `MooseEnum` parameter is retrieved, it can be compared directly to strings or converted back
to a C++ enum with `getEnum<T>()`:

```cpp
if (getParam<MooseEnum>("point_not_found_behavior") == "ERROR")
  mooseError("Unable to locate point.");

const auto behavior =
    getParam<MooseEnum>("point_not_found_behavior").getEnum<PointNotFoundBehavior>();
```

## `MultiMooseEnum`

`MultiMooseEnum` is the multi-select variant. It uses the same valid-name definition, but stores
zero or more active values:

```cpp
params.addParam<MultiMooseEnum>("extra_symbols",
                                MultiMooseEnum("x y z t dt"),
                                "Special symbols to make available.");
```

The selected values can then be queried with methods such as `contains()` or iterated over.

## `CreateMooseEnumClass`

When code needs both a C++ `enum class` and a matching `MooseEnum` option string, the
`CreateMooseEnumClass` macro in [`framework/include/utils/MooseEnum.h`](/framework/include/utils/MooseEnum.h)
keeps those declarations in one place.

For example:

```cpp
CreateMooseEnumClass(PointNotFoundBehavior, ERROR, WARNING, IGNORE);
```

This expands to:

- an `enum class PointNotFoundBehavior`
- a `getPointNotFoundBehaviorOptions()` helper returning the corresponding option string for
  `MooseEnum` or `MultiMooseEnum`

That helper can then be used directly in parameter declarations:

```cpp
params.addParam<MooseEnum>("point_not_found_behavior",
                           MooseEnum(getPointNotFoundBehaviorOptions(), "IGNORE"),
                           "Behavior to use when a point cannot be located.");
```

This pattern prevents the enum declaration and the valid input string from drifting apart during
maintenance. Explicit integer assignments are also supported:

```cpp
CreateMooseEnumClass(ConstantTypeEnum, NONE = 0, ELEMENT = 1, SUBDOMAIN = 2);
```

The generated option helper will preserve those explicit values in the returned `MooseEnum` option
string.
