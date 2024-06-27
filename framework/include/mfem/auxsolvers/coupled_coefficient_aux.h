#pragma once
#include "auxsolver_base.h"

// Specify classes that perform auxiliary calculations on GridFunctions or
// Coefficients.
namespace platypus
{

// Class to allow creation of scalar coefficients that are coupled to
// gridfunctions. Defines a Coefficient that is updated from a GridFunction (to
// be moved into Coefficients?)
class CoupledCoefficient : public mfem::Coefficient, public AuxSolver
{
protected:
  mfem::ParGridFunction * _gf{nullptr};
  double _scalar_val;

public:
  CoupledCoefficient(const platypus::InputParameters & params);

  ~CoupledCoefficient() override = default;

  void Init(const platypus::GridFunctions & gridfunctions,
            platypus::Coefficients & coefficients) override;

  double Eval(mfem::ElementTransformation & T, const mfem::IntegrationPoint & ip) override;

  void Solve(double t = 0.0) override {}

  std::string _coupled_var_name; // name of the variable
};

} // namespace platypus
