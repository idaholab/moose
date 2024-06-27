#include "vector_gridfunction_cross_product_aux.hpp"

#include <utility>

namespace hephaestus
{

void
cross_product(mfem::Vector & va, mfem::Vector & vb, mfem::Vector & V)
{
  V.SetSize(3);
  V[0] = va[1] * vb[2] - va[2] * vb[1];
  V[1] = va[2] * vb[0] - va[0] * vb[2];
  V[2] = va[0] * vb[1] - va[1] * vb[0];
}

void
VectorGridFunctionCrossProductCoefficient::Eval(mfem::Vector & uxv,
                                                mfem::ElementTransformation & T,
                                                const mfem::IntegrationPoint & ip)
{
  mfem::Vector u_vec, v_vec;
  _u_gf.GetVectorValue(T, ip, u_vec);
  _v_gf.GetVectorValue(T, ip, v_vec);
  hephaestus::cross_product(u_vec, v_vec, uxv);
}

VectorGridFunctionCrossProductAux::VectorGridFunctionCrossProductAux(
    const std::string & cross_product_gf_name,
    const std::string & cross_product_coef_name,
    std::string u_gf_name,
    std::string v_gf_name)
  : VectorCoefficientAux(cross_product_gf_name, cross_product_coef_name),
    _u_gf_name(std::move(u_gf_name)),
    _v_gf_name(std::move(v_gf_name))

{
}

void
VectorGridFunctionCrossProductAux::Init(const hephaestus::GridFunctions & gridfunctions,
                                        hephaestus::Coefficients & coefficients)
{
  _u_gf = gridfunctions.Get(_u_gf_name);
  _v_gf = gridfunctions.Get(_v_gf_name);

  coefficients._vectors.Register(
      _vec_coef_name,
      std::make_shared<hephaestus::VectorGridFunctionCrossProductCoefficient>(*_u_gf, *_v_gf));

  VectorCoefficientAux::Init(gridfunctions, coefficients);
}

} // namespace hephaestus
