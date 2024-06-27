#include "rwte10_port_rbc.hpp"

namespace hephaestus
{

RWTE10PortRBC::RWTE10PortRBC(const std::string & name_,
                             mfem::Array<int> bdr_attributes_,
                             double frequency_,
                             double port_length_vector_[3],
                             double port_width_vector_[3],
                             bool input_port_)
  : RobinBC(name_, bdr_attributes_, nullptr, nullptr, nullptr, nullptr),
    _input_port(input_port_),
    _omega(2 * M_PI * frequency_),
    _a1_vec(port_length_vector_, 3),
    _a2_vec(port_width_vector_, 3),
    _a3_vec(CrossProduct(_a1_vec, _a2_vec)),
    _a2xa3(CrossProduct(_a2_vec, _a3_vec)),
    _a3xa1(CrossProduct(_a3_vec, _a1_vec)),
    _v(mfem::InnerProduct(_a1_vec, _a2xa3)),
    _kc(M_PI / _a1_vec.Norml2()),
    _k0(_omega * sqrt(epsilon0_ * mu0_)),
    _k(std::complex<double>(0., sqrt(_k0 * _k0 - _kc * _kc))),
    _k_a(_a2xa3),
    _k_c(_a3_vec)
{
  _k_a *= M_PI / _v;
  _k_c *= _k.imag() / _a3_vec.Norml2();

  _robin_coef_im = std::make_unique<mfem::ConstantCoefficient>(_k.imag() / mu0_);
  _blfi_im = std::make_unique<mfem::VectorFEMassIntegrator>(_robin_coef_im.get());

  if (_input_port)
  {
    _u_real = std::make_unique<mfem::VectorFunctionCoefficient>(
        3, [this](const mfem::Vector & x, mfem::Vector & v) { return RWTE10Real(x, v); });

    _u_imag = std::make_unique<mfem::VectorFunctionCoefficient>(
        3, [this](const mfem::Vector & x, mfem::Vector & v) { return RWTE10Imag(x, v); });

    _lfi_re = std::make_unique<mfem::VectorFEBoundaryTangentLFIntegrator>(*_u_real);
    _lfi_im = std::make_unique<mfem::VectorFEBoundaryTangentLFIntegrator>(*_u_imag);
  }
}

void
RWTE10PortRBC::RWTE10(const mfem::Vector & x, std::vector<std::complex<double>> & E)
{

  mfem::Vector e_hat(CrossProduct(_k_c, _k_a));
  e_hat *= 1.0 / e_hat.Norml2();

  double e0(sqrt(2 * _omega * mu0_ / (_a1_vec.Norml2() * _a2_vec.Norml2() * _k.imag())));
  std::complex<double> e_mag = e0 * sin(InnerProduct(_k_a, x)) * exp(-zi * InnerProduct(_k_c, x));

  E[0] = e_mag * e_hat(1);
  E[1] = e_mag * e_hat(2);
  E[2] = e_mag * e_hat(0);
}

void
RWTE10PortRBC::RWTE10Real(const mfem::Vector & x, mfem::Vector & v)
{
  std::vector<std::complex<double>> eval(x.Size());
  RWTE10(x, eval);
  for (int i = 0; i < x.Size(); ++i)
  {
    v(i) = -2 * _k.imag() * eval[i].imag() / mu0_;
  }
}
void
RWTE10PortRBC::RWTE10Imag(const mfem::Vector & x, mfem::Vector & v)
{
  std::vector<std::complex<double>> eval(x.Size());
  RWTE10(x, eval);
  for (int i = 0; i < x.Size(); ++i)
  {
    v(i) = 2 * _k.imag() * eval[i].real() / mu0_;
  }
}

} // namespace hephaestus
