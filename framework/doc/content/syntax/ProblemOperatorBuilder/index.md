# MFEM Problem Operator Builder System
The `ProblemOperatorBuilder` is a builder/adapter class that is used for constructing
problem-operators with custom inputs. This class is specifically intended for building
user defined custom operators which may be raw `mfem::Operator`'s optimised for specific
purposes e.g. MHD with customised inputs. The Operators may need thin layer access to the MOOSE
multi-physics system. As of yet only a single problem operator per MFEM problem is used, even if
multiple are defined. The ProblemOperatorBuilder classes are built within the MFEMProblem class 
however the ProblemOperatos are built and owned by the MFEM executioners.

## Steady problem operator builder
The ProblemOperatorBuilderSteady class is the default of the MFEMSteady executioner and does
not need to be declared explicitly. The `ProblemOperatorBuilderSteady` class uses	 
systematic logic to build one of 3 possible ProblemOperators they are the `EquationSystemProblemOperator`
, `ComplexEquationSystemProblemOperator` and `EigenproblemESProblemOperator`.

## Transient problem operator builder
The ProblemOperatorBuilderTransient class is the default of the MFEMTransient executioner and does
not need to be declared explicitly. The `ProblemOperatorBuilderTransient` class builds the 
`TimeDependentEquationSystemProblemOperator`.

## Custom problem operator builder example
The custom problem operator example will follow the `MFEMCustomProblemOperator.C` unit-test
which is based on MFEM's [ex0p](https://github.com/mfem/mfem/blob/master/examples/ex0p.cpp).
Firstly a CustomProblemOperator class must be built, if the problem is steady i.e called from
the MFEMSteady executioner (as in this example) then the `ProblemOperator` class inherits from 
`Moose::MFEM::ProblemOperator`. If the CustomProblemOperator class is transient i.e. called from
the MFEMTransient executioner then the class must inherit from `Moose::MFEM::TimeDependentProblemOperator`.
For this case the problem is steady and an example class may look as follows:

```
class CustomDummyProblemOperator : public Moose::MFEM::ProblemOperator
{
  private:
  .
  .
  .
  public:
    // Constructor
    CustomDummyProblemOperator(MFEMProblem & prob, ...);

    // Solve the equation
    virtual void Solve() override;

    // Mult by the operator
    void Mult(const mfem::Vector &, mfem::Vector y) const override {};
};

```

As the problemOperator is built by the MFEM executioners all registered entities in `MFEMProblem`
have been built e.g. the NlSolver, the lSolver, the FEspaces, the ParGridFunctions etc. A custom
operator builder function should be added in the case of non-linear problems where the operator is
rebuilt. In the non-linear case the user would have to override and populate the `Mult` and
the `GetGradient` functions to rebuild, fetch and apply the operators for the residual and
jacobian. The self pointer would have be passed to the Non-linear solver via the 
`SetOperator(mfem::Operator &)` method in the non-linear case. The case in this example is 
linear and the Operator is simply built in the constructor. Firstly the class needs `Form`'s,
`Coefficient`'s, BC `Array`s, `Operator`s and solution/forcing `Vector`s.

```
class CustomDummyProblemOperator : public Moose::MFEM::ProblemOperator
{
  private:
    // Linear and bilinear forms
    mfem::ParBilinearForm * a;
    mfem::ParLinearForm * b;

    // Coefficient(s)
    mfem::ConstantCoefficient one;

    // Boundary condition dofs
    mfem::Array<int> boundary_dofs;

    //Operator and solution vectors
    mfem::OperatorHandle probOp;
    mfem::Vector B, X;

  public:
    // Constructor
    CustomDummyProblemOperator(MFEMProblem & prob0);

    // Solve the equation
    virtual void Solve() override;

    // Mult by the operator
    void Mult(const mfem::Vector &, mfem::Vector y) const override {};
};

```

Once the member variables are declared, the next thing is to set-up the problem. The
`Forms` need access to the FE-Spaces and the post processors need access to the `GridFunction`s.
Both the FE-Spaces and `GridFunction`s are owned by the `MFEMProblem` and need to be retrieved
for usage in the `ProblemOperator`, assuming for this example that the FE-Space and `GridFunction`
that we are interested in have an expected name then the constructor may look like this:

```
CustomDummyProblemOperator::CustomDummyProblemOperator(MFEMProblem & prob0)
{
  // Retrieve the FE-space and gridFunction
  const std::string FEspaceName = "h1";
  const std::string gFuncName = "var0";
  auto fes = prob0.getProblemData().fespaces.GetShared(FEspaceName);
  auto gfunc = prob0.getProblemData().gridfunctions.GetShared(gFuncName);
  .
  .
  .
};
```
The rest of the constructor mirrors the MFEM ex0p example, i.e. build the forms, add the
integrators, assemble the forms and form the linear system:

```
CustomDummyProblemOperator::CustomDummyProblemOperator(MFEMProblem & prob0)
  : Moose::MFEM::ProblemOperator(prob0)
{
  // Retrieve the FE-space and gridFunction
  const std::string FEspaceName = "h1";
  const std::string gFuncName = "var0";
  auto fes = prob0.getProblemData().fespaces.GetShared(FEspaceName);
  auto gfunc = prob0.getProblemData().gridfunctions.GetShared(gFuncName);

  // Boundary conditions
  *gfunc = 0.00;
  fes->GetBoundaryTrueDofs(boundary_dofs);

  // Build the linear form
  b = new mfem::ParLinearForm(&(*fes));
  b->AddDomainIntegrator(new mfem::DomainLFIntegrator(one));
  b->Assemble();

  // Build the bilinear form
  a = new mfem::ParBilinearForm(&(*fes));
  a->AddDomainIntegrator(new mfem::DiffusionIntegrator);
  a->Assemble();

  // Form the linear system
  a->FormLinearSystem(boundary_dofs, *gfunc, *b, probOp, X, B);
}
```

The solve method solves the linear/non-linear system that has been setup and passes the
data to the mfem `GridFunctions` so that the post-processors can view the results.
During construction as the `Moose::MFEM::ProblemOperator` class was constructed as well,
the class inherits a reference to the `MFEMProblem` namely `_problem_data` it can be used
to acces `MFEMProblem` members.

```
void CustomDummyProblemOperator::Solve() override
{
  // Set the operator and solve the equation
  _problem_data.jacobian_solver->SetOperator(*probOp);
  _problem_data.jacobian_solver->GetSolver().Mult(B, X);

  // Set the data in the grid function
  const std::string gFuncName = "var0";
  auto gfunc = _problem_data.gridfunctions.GetShared(gFuncName);
  gfunc->SetFromTrueDofs(X);
};

```

Once the `ProblemOperator` has been built the `ProblemOperatorBuilder` class is needed,
this class is constructed by `MFEMProblem` and called by the executioner. The builder class
must inherit from `ProblemOperatorBuilderBase` making it an `MFEMObject` and by proxy a 
`MooseObject` thus it has a fixed signature constructor and destructor, it has one method
that has a fixed signature that is called by the executioner. All of the code must
be within the `Moose::MFEM` namespace for simplicity. An example minimal class looks like:

```
namespace Moose::MFEM
{
class ProblemOperatorBuilderCustomDummy : public ProblemOperatorBuilderBase
{
private:
  type1 param1;
  .
  .
  .

public:
  static InputParameters validParams();

  ProblemOperatorBuilderCustomDummy(const InputParameters & parameters);

  ~ProblemOperatorBuilderCustomDummy() = default;

  /// Returns a pointer to the operator's equation system.
  std::shared_ptr<Moose::MFEM::ProblemOperatorBase>
  createProblemOperator(MFEMProblem & mfemProb) override;
};
};
```

The `validParams()` method can be used to generate custom inputs for the problem operator,
the inputs can be then put in the ProblemOperatorBuilder block of the input files.
```
namespace Moose::MFEM
{
InputParameters ProblemOperatorBuilderCustomDummy::validParams()
{
  InputParameters params = ProblemOperatorBuilderBase::validParams();
  params.addParam<type1>("name", "defaultValue", "Description of param");
  .
  .
  .
  return params;
}
};
```

The constructor can be left more or less empty if the operator being built has no custom
options associated with it directly (e.g. solver customisations may occur in the `MFEMProblem`
class), but in the case there are custom inputs it can be used to retrieve the input params
and store them.
```
namespace Moose::MFEM
{
ProblemOperatorBuilderCustomDummy::ProblemOperatorBuilderCustomDummy(const InputParameters & parameters)
  : ProblemOperatorBuilderBase(parameters) 
{
  param1 = getParam<type1>("name");
  .
  .
  .
}
};
```

The last method to be built is the `createProblemOperator` it simply returns a shared pointer
to the ProblemOperator that was defined earlier, this method is called by the executioner,
and the resulting object is owned by the executioner.
```
namespace Moose::MFEM
{
std::shared_ptr<Moose::MFEM::ProblemOperatorBase>
ProblemOperatorBuilderCustomDummy::createProblemOperator(MFEMProblem & mfemProb) override
{
  return std::make_shared<CustomDummyProblemOperator>(mfemProb, param1, ...);
}
};
```

Once the object has been defined, the new `ProblemOperatorBuilder` object must be registered
to the MooseApp system:
```
namespace Moose::MFEM
{
  registerMooseObject("MooseApp", ProblemOperatorBuilderCustomDummy);
};
```
In the unit test example the complete code looks like this:
```
namespace Moose::MFEM
{
class ProblemOperatorBuilderCustomDummy : public ProblemOperatorBuilderBase
{
public:
  static InputParameters validParams()
  {
    InputParameters params = ProblemOperatorBuilderBase::validParams();
    return params;
  };

  ProblemOperatorBuilderCustomDummy(const InputParameters & parameters)
    : ProblemOperatorBuilderBase(parameters) {};

  ~ProblemOperatorBuilderCustomDummy() = default;

  /// Returns a pointer to the operator's equation system.
  std::shared_ptr<Moose::MFEM::ProblemOperatorBase>
  createProblemOperator(MFEMProblem & mfemProb) override
  {
    return std::make_shared<CustomDummyProblemOperator>(mfemProb);
  };
};

registerMooseObject("MooseApp", ProblemOperatorBuilderCustomDummy);
};
```
