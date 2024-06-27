#pragma once
#include "vector_coefficient_aux.hpp"

// Specify postprocessors that depend on one or more gridfunctions
namespace hephaestus
{

void cross_product(mfem::Vector & va, mfem::Vector & vb, mfem::Vector & V);

// The VectorGridFunctionDotProductCoefficient evaluates the cross product of
// two gridfunctions
class VectorGridFunctionCrossProductCoefficient : public mfem::VectorCoefficient
{
private:
  mfem::ParGridFunction & _u_gf;
  mfem::ParGridFunction & _v_gf;

public:
  VectorGridFunctionCrossProductCoefficient(mfem::ParGridFunction & u_gf,
                                            mfem::ParGridFunction & v_gf)
    : mfem::VectorCoefficient(3), _u_gf(u_gf), _v_gf(v_gf)
  {
  }

  ~VectorGridFunctionCrossProductCoefficient() override = default;

  void Eval(mfem::Vector & uxv,
            mfem::ElementTransformation & T,
            const mfem::IntegrationPoint & ip) override;
};

// Auxsolver to project the cross product of two vector gridfunctions onto a
// third (vector) GridFunction
class VectorGridFunctionCrossProductAux : public VectorCoefficientAux
{
private:
  mfem::ParGridFunction * _v_gf{nullptr};
  mfem::ParGridFunction * _u_gf{nullptr};

  const std::string _u_gf_name;
  const std::string _v_gf_name;

public:
  VectorGridFunctionCrossProductAux(const std::string & cross_product_gf_name,
                                    const std::string & cross_product_coef_name,
                                    std::string u_gf_name,
                                    std::string v_gf_name);

  void Init(const hephaestus::GridFunctions & gridfunctions,
            hephaestus::Coefficients & coefficients) override;
};

} // namespace hephaestus
