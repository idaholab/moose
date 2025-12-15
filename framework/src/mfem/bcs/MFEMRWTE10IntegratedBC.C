//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMRWTE10IntegratedBC.h"

registerMooseObject("MooseApp", MFEMRWTE10IntegratedBC);

InputParameters
MFEMRWTE10IntegratedBC::validParams()
{
  InputParameters params = MFEMComplexIntegratedBC::validParams();
  params.addClassDescription("Adds the Robin boundary conditions for an electromagnetic problem "
                             "with transverse waves in a rectangular waveguide.");
  params.addParam<RealVectorValue>(
      "port_length_vector",
      "Vector along the x-axis of the port, where its magnitude is the port length in meters.");
  params.addParam<RealVectorValue>(
      "port_width_vector",
      "Vector along the y-axis of the port, where its magnitude is the port width in meters.");
  params.addParam<Real>("frequency", 1.0, "Mode frequency in Hz.");
  params.addParam<Real>("epsilon", 1.0, "Electric permittivity constant.");
  params.addParam<Real>("mu", 1.0, "Magnetic permeability constant.");
  params.addParam<bool>("input_port",
                        false,
                        "Whether the boundary attribute passed to this BC corresponds to the input "
                        "port of the waveguide.");

  return params;
}

MFEMRWTE10IntegratedBC::MFEMRWTE10IntegratedBC(const InputParameters & parameters)
  : MFEMComplexIntegratedBC(parameters),
    _mu(getParam<Real>("mu")),
    _epsilon(getParam<Real>("epsilon")),
    _omega(2 * M_PI * getParam<Real>("frequency")),
    _a1(const_cast<mfem::real_t *>(&getParam<RealGradient>("port_length_vector")(0)), Moose::dim),
    _a2(const_cast<mfem::real_t *>(&getParam<RealGradient>("port_width_vector")(0)), Moose::dim),
    _k0(_omega * std::sqrt(_mu * _epsilon)),
    _kc(M_PI / _a1.Norml2()),
    _k(std::complex<mfem::real_t>(0., std::sqrt(_k0 * _k0 - _kc * _kc))),
    _k_c(normalizedCrossProduct(_a1, _a2) *= _k.imag()),
    _k_a(normalizedCrossProduct(_a2, _k_c) *= _kc)
{
  _robin_coef_im = std::make_unique<mfem::ConstantCoefficient>(_k.imag() / _mu);

  if (getParam<bool>("input_port"))
  {
    _u_real = std::make_unique<mfem::VectorFunctionCoefficient>(
        3, [this](const mfem::Vector & x, mfem::Vector & v) { return RWTE10Real(x, v); });

    _u_imag = std::make_unique<mfem::VectorFunctionCoefficient>(
        3, [this](const mfem::Vector & x, mfem::Vector & v) { return RWTE10Imag(x, v); });
  }
}

void
MFEMRWTE10IntegratedBC::RWTE10(const mfem::Vector & x, std::vector<std::complex<mfem::real_t>> & E)
{

  mfem::Vector e_hat(normalizedCrossProduct(_k_c, _k_a));
  std::complex<mfem::real_t> zi(0., 1.);

  mfem::real_t e0 = std::sqrt(2 * _omega * _mu / (_a1.Norml2() * _a2.Norml2() * _k.imag()));
  std::complex<mfem::real_t> e_mag =
      e0 * sin(mfem::InnerProduct(_k_a, x)) * exp(-zi * mfem::InnerProduct(_k_c, x));

  E[0] = e_mag * e_hat(1);
  E[1] = e_mag * e_hat(2);
  E[2] = e_mag * e_hat(0);
}

void
MFEMRWTE10IntegratedBC::RWTE10Real(const mfem::Vector & x, mfem::Vector & v)
{
  std::vector<std::complex<mfem::real_t>> eval(x.Size());
  RWTE10(x, eval);
  for (int i = 0; i < x.Size(); ++i)
    v(i) = -2 * _k.imag() * eval[i].imag() / _mu;
}
void
MFEMRWTE10IntegratedBC::RWTE10Imag(const mfem::Vector & x, mfem::Vector & v)
{
  std::vector<std::complex<mfem::real_t>> eval(x.Size());
  RWTE10(x, eval);
  for (int i = 0; i < x.Size(); ++i)
    v(i) = 2 * _k.imag() * eval[i].real() / _mu;
}

#endif
