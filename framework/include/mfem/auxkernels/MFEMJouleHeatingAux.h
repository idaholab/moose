#pragma once

#include "MFEMAuxSolver.h"

class JouleHeatingCoefficient : public hephaestus::CoupledCoefficient
{
private:
  mfem::Coefficient * sigma{nullptr};
  mfem::ParGridFunction * joule_heating_gf{nullptr};
  std::string conductivity_coef_name;
  // std::string var_name;

public:
  JouleHeatingCoefficient(const hephaestus::InputParameters & params)
    : hephaestus::CoupledCoefficient(params),
      conductivity_coef_name(params.GetParam<std::string>("ConductivityCoefName"))
  {
  }

  void Init(const hephaestus::GridFunctions & variables, hephaestus::Coefficients & coefficients)
  {
    // To ensure conductivity on subdomains is converted into global coefficient
    // Hephaestus update for coefficients initialisation could address this
    // if (!coefficients.scalars.Has(conductivity_coef_name))
    // {
    //   coefficients.scalars.Register(
    //       conductivity_coef_name,
    //       new mfem::PWCoefficient(
    //           coefficients.getGlobalScalarProperty(std::string(conductivity_coef_name))),
    //       true);
    // }

    hephaestus::CoupledCoefficient::Init(variables, coefficients);
    std::cout << "Intialising JouleHeating";
    sigma = coefficients._scalars.Get(conductivity_coef_name);

    joule_heating_gf = variables.Get("joule_heating");
  }

  virtual double Eval(mfem::ElementTransformation & T, const mfem::IntegrationPoint & ip)
  {
    mfem::Vector E;
    double thisSigma;
    _gf->GetVectorValue(T, ip, E);
    thisSigma = sigma->Eval(T, ip);
    return thisSigma * (E * E);
  }

  void Solve(double t)
  {
    this->SetTime(t);
    joule_heating_gf->ProjectCoefficient(*this);
  }
};

class MFEMJouleHeatingAux : public MFEMAuxSolver
{
public:
  static InputParameters validParams();

  MFEMJouleHeatingAux(const InputParameters & parameters);
  virtual ~MFEMJouleHeatingAux();

  virtual void execute() override {}
  virtual void initialize() override {}
  virtual void finalize() override {}

  inline std::shared_ptr<hephaestus::AuxSolver> getAuxSolver() const override
  {
    return joule_heating_aux;
  }

  virtual void storeCoefficients(hephaestus::Coefficients & coefficients) override;

protected:
  hephaestus::InputParameters joule_heating_params;
  std::shared_ptr<JouleHeatingCoefficient> joule_heating_aux{nullptr};
};
