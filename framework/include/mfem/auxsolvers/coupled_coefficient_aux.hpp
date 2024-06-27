#pragma once
#include "auxsolver_base.hpp"

// Specify classes that perform auxiliary calculations on GridFunctions or
// Coefficients.
namespace hephaestus
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
  CoupledCoefficient(const hephaestus::InputParameters & params);

  ~CoupledCoefficient() override = default;

  void Init(const hephaestus::GridFunctions & gridfunctions,
            hephaestus::Coefficients & coefficients) override;

  double Eval(mfem::ElementTransformation & T, const mfem::IntegrationPoint & ip) override;

  void Solve(double t = 0.0) override {}

  std::string _coupled_var_name; // name of the variable
};

} // namespace hephaestus
