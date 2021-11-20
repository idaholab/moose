# Expression Builder

Build FParser expressions using C++ operator overloading

Mixing in the `ExpressionBuilder` class into your MOOSE classes will allow you to build FParser
expressions using familiar C++ syntax.

## Introduction

### EBTerm

Variables used in your expressions are of type `EBTerm`. The following declares three variables that
can be used in an expression:

```cpp
EBTerm c1("c1"), c2("c3"), phi("phi");
```

### EBFunction

Declare a function `G` and define it:

```cpp
EBFunction G;

G(c1, c2, c3) = c1 + 2 * c2 + 3 * pow(c3, 2);
```

Performing a substitution is as easy as:

```cpp
EBFunction H;

H(c1, c2) = G(c1, c2, 1-c1-c2)
```

Use the `<<` io operator to output functions or terms. Or use explicit or implicit casts from
`EBFunction` to `std::string` to pass a function to the FParse Parse method. FParser variables are
built using the `args()` method.

```cpp
FunctionParserADBase<Real> GParser;

GParser.Parse(G, G.args);
```

## Example

To use `ExpressionBuilder` inherit from it in addition to the base class your free energy material is
based on. A common scenario is to inherit from `DerivativeParsedMaterialHelper` and
`ExpressionBuilder`. The class definition would be:

```cpp
class ExampleFreeEnergy : public DerivativeParsedMaterialHelper,
                          public ExpressionBuilder
{
public:
  ExampleFreeEnergy(const InputParameters & parameters);

protected:
  /// Variables used in the free energy
  EBTerm _T, _c;

  /// Boltzmann constant
  const Real _kB;
};
```

The free energy expression would then be built in the constructor.

```cpp
InputParameters
ExampleFreeEnergy::validParams()
{
  InputParameters params = DerivativeParsedMaterialHelper::validParams();
  params.addClassDescription("Example derivative free energy material");
  params.addRequiredCoupledVar("T", "Temperature");
  params.addRequiredCoupledVar("c", "Concentration");
  return params;
}

ExampleFreeEnergy::ExampleFreeEnergy(const InputParameters & parameters) :
    DerivativeParsedMaterialHelper(parameters),
    _T("T"), // we use the names from validParams for the variables
    _c("c"),
    _kB(8.6173324e-5)
{
  EBFunction G, Grand;

  // configurational entropy contribution
  Grand(_c,_T) = _kB * _T * (_c*log(_c) + (1-_c)*log(1-_c));

  // Total free energy
  G(_c,_T) = _c*_c*(1-_c)*(1-_c) + _Grand(_c,_T);

  functionParse(G);
}
```

That's it. The `functionParse` call will generate, optimize, and (if selected by the user through the
`enable_jit` config parameter) compile the function and its derivatives.

All named variables declared through `EBTerm var("name")` will be substituted by the coupled
variables of the same `name`.

## See also

- [Automatic Differentiation](FunctionMaterials/AutomaticDifferentiation.md) for MOOSE application developers
- [Just In Time Compilation](FunctionMaterials/JITCompilation.md) of parsed functions for MOOSE application developers
