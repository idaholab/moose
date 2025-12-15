//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MFEMComplexIntegratedBC.h"

class MFEMRWTE10IntegratedBC : public MFEMComplexIntegratedBC
{
public:
  static InputParameters validParams();

  MFEMRWTE10IntegratedBC(const InputParameters & parameters);

  void RWTE10(const mfem::Vector & x, std::vector<std::complex<mfem::real_t>> & E);
  void RWTE10Real(const mfem::Vector & x, mfem::Vector & v);
  void RWTE10Imag(const mfem::Vector & x, mfem::Vector & v);

  virtual mfem::LinearFormIntegrator * getRealLFIntegrator() override
  {
    return (getParam<bool>("input_port") ? new mfem::VectorFEBoundaryTangentLFIntegrator(*_u_real)
                                         : nullptr);
  }
  virtual mfem::LinearFormIntegrator * getImagLFIntegrator() override
  {
    return (getParam<bool>("input_port") ? new mfem::VectorFEBoundaryTangentLFIntegrator(*_u_imag)
                                         : nullptr);
  }
  virtual mfem::BilinearFormIntegrator * getRealBFIntegrator() override { return nullptr; }
  virtual mfem::BilinearFormIntegrator * getImagBFIntegrator() override
  {
    return new mfem::VectorFEMassIntegrator(_robin_coef_im.get());
  }

  /// Computes the unit vector in the direction of the cross product of two 3d vectors
  mfem::Vector normalizedCrossProduct(const mfem::Vector & va, const mfem::Vector & vb)
  {
    mfem::Vector vec;
    va.cross3D(vb, vec);
    return vec /= vec.Norml2();
  }

protected:
  mfem::real_t _mu;
  mfem::real_t _epsilon;
  mfem::real_t _omega;

  mfem::Vector _a1;
  mfem::Vector _a2;

  mfem::real_t _k0;
  mfem::real_t _kc;
  std::complex<mfem::real_t> _k;

  mfem::Vector _k_c;
  mfem::Vector _k_a;

  std::unique_ptr<mfem::ConstantCoefficient> _robin_coef_im{nullptr};
  std::unique_ptr<mfem::VectorFunctionCoefficient> _u_real{nullptr};
  std::unique_ptr<mfem::VectorFunctionCoefficient> _u_imag{nullptr};
};

#endif
