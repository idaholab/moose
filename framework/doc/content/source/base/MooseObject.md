# MooseObject

The `MooseObject` is the [top level base class](https://mooseframework.inl.gov/docs/doxygen/moose/classMooseObject.html)
for all non-action MOOSE objects (such as Kernels, Materials, Postprocessors,
etc.).

This class holds the main convenience functions, such as

- `getParam<T>(param)` template as a shortcut to `parameters().get<T>(param)` to get an input parameter `param` of type `T`
- `isParamValid(param)` to check if the input parameter `param` has a value that can be obtained with `getParam`
- `getMooseApp()` to get a reference to the [`MooseApp`](MooseApp.md) this object is associated with
- `type()` to get the registered class object name of the current object
- `name()` to get the name the object appears under in the input file
- `parameters()` to get a reference to the [`InputParameters`](InputParameters.md) of the current object
- `paramError`, `paramWarning`, and `paramInfo` to output status messages regarding specific parameters that are annotated with the input file location of the parameter

## Parameter vector pairs

```C++
  template <typename T1, typename T2>
  std::vector<std::pair<T1, T2>> getParam(const std::string & param1,
                                          const std::string & param2) const;
```

Can be used to fetch two input parameters of type `std::vector<T1>` and
`std::vector<T2>` into a single `std::vector<std::pair<T1, T2>>`. This is useful
for pairs of vectors that have a one to one correspondence, such as
[!param](/Executioner/Steady/petsc_options_iname) and
[!param](/Executioner/Steady/petsc_options_value).
