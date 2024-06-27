#include "vector_gridfunction_dot_product_aux.hpp"

#include <utility>

namespace hephaestus
{

double
VectorGridFunctionDotProductCoefficient::Eval(mfem::ElementTransformation & T,
                                              const mfem::IntegrationPoint & ip)
{
  double coef_value;
  coef_value = _coef.Eval(T, ip);

  mfem::Vector u_re;
  mfem::Vector u_im;
  mfem::Vector v_re;
  mfem::Vector v_im;

  _u_gf_re->GetVectorValue(T, ip, u_re);
  _v_gf_re->GetVectorValue(T, ip, v_re);
  if (_u_gf_im == nullptr || _v_gf_im == nullptr)
  {
    return coef_value * (u_re * v_re);
  }
  else
  {
    _u_gf_im->GetVectorValue(T, ip, u_im);
    _v_gf_im->GetVectorValue(T, ip, v_im);
    return 0.5 * coef_value * (u_re * v_re + u_im * v_im);
  }
}

VectorGridFunctionDotProductAux::VectorGridFunctionDotProductAux(
    const std::string & dot_product_gf_name,
    const std::string & dot_product_coef_name,
    std::string scaling_coef_name,
    std::string u_gf_real_name,
    std::string v_gf_real_name,
    std::string u_gf_imag_name,
    std::string v_gf_imag_name,
    const bool complex_average)
  : CoefficientAux(dot_product_gf_name, dot_product_coef_name),
    _u_gf_real_name(std::move(u_gf_real_name)),
    _v_gf_real_name(std::move(v_gf_real_name)),
    _u_gf_imag_name(std::move(u_gf_imag_name)),
    _v_gf_imag_name(std::move(v_gf_imag_name)),
    _scaling_coef_name(std::move(scaling_coef_name)),
    _complex_average(complex_average)

{
}

void
VectorGridFunctionDotProductAux::Init(const hephaestus::GridFunctions & gridfunctions,
                                      hephaestus::Coefficients & coefficients)
{
  if (_scaling_coef_name.empty())
  {
    _scaling_coef = coefficients._scalars.Get("_one");
  }
  else
  {
    _scaling_coef = coefficients._scalars.Get(_scaling_coef_name);
  }
  if (_complex_average)
  {
    _u_gf_re = gridfunctions.Get(_u_gf_real_name);
    _v_gf_re = gridfunctions.Get(_v_gf_real_name);
    _u_gf_im = gridfunctions.Get(_u_gf_imag_name);
    _v_gf_im = gridfunctions.Get(_v_gf_imag_name);
  }
  else
  {
    _u_gf_re = gridfunctions.Get(_u_gf_real_name);
    _v_gf_re = gridfunctions.Get(_v_gf_real_name);
  }

  coefficients._scalars.Register(_coef_name,
                                 std::make_shared<VectorGridFunctionDotProductCoefficient>(
                                     *_scaling_coef, _u_gf_re, _v_gf_re, _u_gf_im, _v_gf_im));

  CoefficientAux::Init(gridfunctions, coefficients);
}

} // namespace hephaestus
