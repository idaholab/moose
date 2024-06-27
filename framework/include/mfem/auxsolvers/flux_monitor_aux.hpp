#pragma once
#include "auxsolver_base.hpp"

// Specify postprocessors that depend on one or more gridfunctions
namespace hephaestus
{

double calcFlux(mfem::GridFunction * v_field, int face_attr);

double calcFlux(mfem::GridFunction * v_field, int face_attr, mfem::Coefficient & q);

// Class to calculate and store the flux of a vector GridFunction through a surface
// at each timestep, optionally scaled by a coefficient.
class FluxMonitorAux : public AuxSolver
{

public:
  FluxMonitorAux() = default;
  FluxMonitorAux(std::string var_name, int face_attr, std::string coef_name = "");

  ~FluxMonitorAux() override = default;

  void Init(const hephaestus::GridFunctions & gridfunctions,
            hephaestus::Coefficients & coefficients) override;

  void Solve(double t = 0.0) override;

  std::string _var_name;  // name of the vector variable
  std::string _coef_name; // name of the coefficient

  mfem::Array<double> _times;
  mfem::Array<double> _fluxes;

  mfem::ParGridFunction * _gf{nullptr};
  mfem::Coefficient * _coef{nullptr};
  int _face_attr;
};

} // namespace hephaestus
