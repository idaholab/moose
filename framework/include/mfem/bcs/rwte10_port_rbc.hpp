#pragma once
#include "robin_bc_base.hpp"

namespace hephaestus
{

class RWTE10PortRBC : public RobinBC
{
  inline static const double epsilon0_{8.8541878176e-12};
  inline static const double mu0_{4.0e-7 * M_PI};

  inline static const std::complex<double> zi{std::complex<double>(0., 1.)};

public:
  RWTE10PortRBC(const std::string & name_,
                mfem::Array<int> bdr_attributes_,
                double frequency,
                double port_length_vector[3],
                double port_width_vector[3],
                bool input_port);

  static mfem::Vector CrossProduct(mfem::Vector & va, mfem::Vector & vb)
  {
    mfem::Vector vec;
    vec.SetSize(3);
    vec[0] = va[1] * vb[2] - va[2] * vb[1];
    vec[1] = va[2] * vb[0] - va[0] * vb[2];
    vec[2] = va[0] * vb[1] - va[1] * vb[0];
    return vec;
  }
  void RWTE10(const mfem::Vector & x, std::vector<std::complex<double>> & E);
  void RWTE10Real(const mfem::Vector & x, mfem::Vector & v);
  void RWTE10Imag(const mfem::Vector & x, mfem::Vector & v);

  bool _input_port;
  double _omega;
  mfem::Vector _a1_vec;
  mfem::Vector _a2_vec;
  mfem::Vector _a3_vec;
  mfem::Vector _a2xa3;
  mfem::Vector _a3xa1;

  double _v;
  double _kc;
  double _k0;
  std::complex<double> _k;

  mfem::Vector _k_a;
  mfem::Vector _k_c;

  std::unique_ptr<mfem::ConstantCoefficient> _robin_coef_im;
  std::unique_ptr<mfem::VectorFunctionCoefficient> _u_real;
  std::unique_ptr<mfem::VectorFunctionCoefficient> _u_imag;
};

} // namespace hephaestus
