# IndexableProperty

The `IndexableProperty` is a helper (proxy) object to obtain a scalar component
from a material property. Use it in objects that process a scalar quantity
instead of a `Real` material property to allow the user to supply any material
property of a type from the list below along with a component index parameter to
select a scalar component from the property value.

| Material property type | `component` parameter example |
| - | - |
|`Real` | _none_ |
|`RealVectorValue` | `component = 1`|
|`std::vector<Real>` | `component = 7`|
|`RankTwoTensor` | `component = "0 2"`|
|`RankThreeTensor` | `component = "1 2 0"`|
|`RankFourTensor` | `component = "1 0 1 2"`|

To use this utility class declare a member in your class header

```c++
const IndexableProperty<Parent, is_ad> _prop;
```

where `Parent` is the current class' parent class. `IndexableProperty` provides
a  helper method to add the default parameters for coupling a material property
(`property`) and its component index (`component`). Use it as follows

```c++
InputParameters params = IndexableProperty<Parent, is_ad>::validParams();
```

This also adds the `Parent` class parameters to `params`. You may also choose to
add your own parameters of type `MaterialPropertyName` and `std::vector<unsigned int>`
with custom names and documentation strings.

```c++
params.addRequiredParam<MaterialPropertyName>("first_property",
                                              "The name of the first material property");
params.addRequiredParam<MaterialPropertyName>("second_property",
                                              "The name of the second material property");
params.addParam<std::vector<unsigned int>>(
    "first_component", "Index of the comonent of the first property");
params.addParam<std::vector<unsigned int>>(
    "second_component", "Index of the comonent of the second property");
```

To initialize the `IndexableProperty` object put

```c++
  _prop(this)
```

in your objects initializer list. If you chose custom names for the property
name and component parameters, pass them in as the second and third arguments,
like so

```
  _first_prop(this, "first_property", "first_component")
  _second_prop(this, "second_property", "second_component")
```

An `IndexableProperty` object can be implicitly cast to `bool` to check if _any_
of the supported indexable properties was found.

```
if (!_prop)
  paramError("property", "No suitable material property was found");
```
