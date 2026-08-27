# ProblemComposers System

!if! function=hasCapability('mfem')

The `ProblemComposers` system allows the user to construct
problem operators with custom inputs. This class is specifically intended for composing
user defined custom operators which may be raw `mfem::Operator`'s optimised for specific
purposes e.g. MHD with customised inputs. The operators may need thin layer access to the MOOSE
multi-physics system. As of yet, only a single problem operator object per [MFEMProblem.md] is used (even if
multiple classes are defined). The [problem composer](MFEMProblemComposer.md) classes are built within the [MFEMProblem.md] class 
however the [ProblemOperator.md]s are built and owned by the MFEM executioners.

## Using a custom problem composer to plug in a custom problem operator

This custom problem operator example will follow the `MFEMCustomProblemOperator.C` unit test which
is based on MFEM's [ex0p](https://github.com/mfem/mfem/blob/master/examples/ex0p.cpp). Firstly, a
custom [ProblemOperator.md] class must be built. If the problem is steady, i.e called from the [MFEMSteady.md]
executioner, then the problem operator class inherits from `Moose::MFEM::ProblemOperator`. If the
problem is transient, i.e. called from the [MFEMTransient.md] executioner, then
the class must inherit from `Moose::MFEM::TimeDependentProblemOperator`. For this case, the problem
is steady and an example class may look as follows:

```cpp
class CustomDummyProblemOperator : public Moose::MFEM::ProblemOperator
{
  private:
  .
  .
  .
  public:
    // Constructor
    CustomDummyProblemOperator(MFEMProblem & prob, ...);

    // Set up the problem
    virtual void Init(mfem::BlockVector) override;

    // Solve the equation
    virtual void Solve() override;

    // Apply the operator
    void Mult(const mfem::Vector & x, mfem::Vector & y) const override {};
};

```

A custom operator builder function should be added in the case of non-linear problems where the
operator is rebuilt. In the non-linear case the user would have to override and populate the
`Mult` and the `GetGradient` functions to rebuild, fetch and apply the operators for the residual
and Jacobian. The self pointer would have be passed to the non-linear solver via the
`SetOperator(mfem::Operator &)` method in the non-linear case. The case in this example is linear
and the Operator is only built once in `Init` function or in the class constructor. Firstly the 
class needs `Form`'s, `Coefficient`'s, BC `Array`s, `Operator`s and solution/forcing `Vector`s, 
an equation system can also be used instead of or along side the previous objects.

```cpp
class CustomDummyProblemOperator : public Moose::MFEM::ProblemOperator
{
  private:
    int _dumm_var = 0;
    // The linear and bilinear forms
    mfem::ParBilinearForm * _a;
    mfem::ParLinearForm * _b;

    // The coefficient
    mfem::ConstantCoefficient _one;

    // The boundary conditions arrays
    mfem::Array<int> _boundary_dofs;

    // The operator and solution vectors (could
    // potentially use the ones in the base class)
    mfem::OperatorHandle _problem_operator; // The actual mfem problem operator
    mfem::Vector _B, _X;

  public:
    // Constructor
    CustomDummyProblemOperator(MFEMProblem & prob0);

    // Set up the problem
    virtual void Init(mfem::BlockVector) override;

    // Solve the equation
    virtual void Solve() override;

    // Mult by the operator
    void Mult(const mfem::Vector & x, mfem::Vector & y) const override {};
};

```

Once the member variables are declared, the next thing is to set-up the problem. The
`Forms` need access to the FE-Spaces and the post processors need access to the `GridFunction`s.
Both the FE-Spaces and `GridFunction`s are owned by the `MFEMProblem` and need to be retrieved
for usage in the `ProblemOperator`, assuming for this example that the FE-Space and `GridFunction`s
that we are interested in have an expected name. The constructor should only be used to retrieve custom
input parameters, retrieveing FE-Spaces and GridFunctions has to be done in the `Init`.

```cpp
CustomDummyProblemOperator::CustomDummyProblemOperator(MFEMProblem & prob0):
  : Moose::MFEM::ProblemOperator(prob0), ...
{}

CustomDummyProblemOperator::Init(mfem::BlockVector &)
{
  // Retrieve the FE-space and gridFunction
  const std::string fe_space_name = "h1";
  const std::string grid_function_name = "var0";
  auto fes = prob0.getProblemData().fespaces.GetShared(fe_space_name);
  auto grid_function = prob0.getProblemData().gridfunctions.GetShared(grid_function_name);
  .
  .
  .
};
```

The rest of the `Init` function mirrors the MFEM ex0p example, i.e. build the forms, add the
integrators, assemble the forms and form the linear system:

```cpp
CustomDummyProblemOperator::CustomDummyProblemOperator(MFEMProblem & prob0)
  : Moose::MFEM::ProblemOperator(prob0), _one(1.000)
{}

CustomDummyProblemOperator::Init(mfem::BlockVector &)
{
  // Retrieve the FE-space and gridFunction
  const std::string fe_space_name = "h1";
  const std::string grid_function_name = "var0";
  auto fes = prob0.getProblemData().fespaces.GetShared(fe_space_name);
  auto grid_function = prob0.getProblemData().gridfunctions.GetShared(grid_function_name);

  // Boundary conditions
  *grid_function = 0.00;
  fes->GetBoundaryTrueDofs(_boundary_dofs);

  // Build the linear form
  _b = new mfem::ParLinearForm(&(*fes));
  _b->AddDomainIntegrator(new mfem::DomainLFIntegrator(_one));
  _b->Assemble();

  // Build the bilinear form
  _a = new mfem::ParBilinearForm(&(*fes));
  _a->AddDomainIntegrator(new mfem::DiffusionIntegrator);
  _a->Assemble();

  // Form the linear system
  _a->FormLinearSystem(_boundary_dofs, *grid_function, *_b, _problem_operator, _X, _B);
}
```

The solve method solves the linear/non-linear system that has been setup and passes the
data to the mfem `GridFunctions` so that the post-processors can view the results.
The class inherits a reference to the `MFEMProblem` and `MFEMProblemData` from
`Moose::MFEM::ProblemOperator` meaning the they can be used to access the grid-functions
and solvers.

```cpp
void CustomDummyProblemOperator::Solve() override
{
  // Set the operator and solve the equation
  _problem_data.jacobian_solver->SetOperator(*_problem_operator);
  _problem_data.jacobian_solver->GetSolver().Mult(_B, _X);

  // Set the data in the grid function
  const std::string grid_function_name = "var0";
  auto grid_function = _problem_data.gridfunctions.GetShared(grid_function_name);
  grid_function->SetFromTrueDofs(_X);
};

```

Once the `ProblemOperator` has been written, an `MFEMProblemComposer` class is needed. The
composer class must inherit from `MFEMProblemComposer` making it an `MFEMObject` and by proxy a
`MooseObject` thus it has a fixed signature constructor and destructor, it has one method
that has a fixed signature. An example minimal class looks like:

```cpp
class CustomDummyProblemComposer : public MFEMProblemComposer
{
private:
  type1 param1;
  .
  .
  .

public:
  static InputParameters validParams();

  CustomDummyProblemComposer(const InputParameters & parameters);

  ~CustomDummyProblemComposer() = default;

  /// Returns a pointer to the problem operator.
  std::shared_ptr<Moose::MFEM::ProblemOperatorBase>
  createProblemOperator(MFEMProblem & mfem_problem) override;
};
```

The `validParams()` method can be used to generate custom inputs for the problem composer,
the inputs can be then put in the `ProblemComposers` block of the input files. The custom inputs
in problem composer can then be passed to the problem operator in the `createProblemOperator` method.

```cpp
InputParameters CustomDummyProblemComposer::validParams()
{
  InputParameters params = MFEMProblemComposer::validParams();
  params.addParam<type1>("name", "default_value", "description of param");
  .
  .
  .
  return params;
}
```

The constructor can be left more or less empty if the operator being built has no custom
options associated with it directly (e.g. solver customisations may occur in the `MFEMProblem`
class), but in the case there are custom inputs it can be used to retrieve the input params
and store them.

```cpp
CustomDummyProblemComposer::CustomDummyProblemComposer(
  const InputParameters & parameters)
  : MFEMProblemComposer(parameters) 
{
  param1 = getParam<type1>("name");
  .
  .
  .
}
```

The last method to be built is the `createProblemOperator` it simply returns a shared pointer
to the `ProblemOperator` that was defined earlier.

```cpp
std::shared_ptr<Moose::MFEM::ProblemOperatorBase>
CustomDummyProblemComposer::createProblemOperator(MFEMProblem & mfemProb) override
{
  return std::make_shared<CustomDummyProblemOperator>(mfemProb, param1, ...);
}
```

Once the object has been defined, the new `MFEMProblemComposer` object must be registered
to the MooseApp system:

```cpp
registerMooseObject("MooseApp", CustomDummyProblemComposer);
```
In the unit test example the complete code looks like this:
```cpp
class CustomDummyProblemComposer : public MFEMProblemComposer
{
public:
  static InputParameters validParams()
  {
    InputParameters params = MFEMProblemComposer::validParams();
    return params;
  };

  CustomDummyProblemComposer(const InputParameters & parameters)
    : MFEMProblemComposer(parameters) {};

  ~CustomDummyProblemComposer() = default;

  /// Returns a pointer to the operator's equation system.
  std::shared_ptr<Moose::MFEM::ProblemOperatorBase>
  createProblemOperator(MFEMProblem & mfem_problem) override
  {
    return std::make_shared<CustomDummyProblemOperator>(mfem_problem);
  };
};

registerMooseObject("MooseApp", CustomDummyProblemComposer);
```
!if-end!

!else
!include mfem/mfem_warning.md
