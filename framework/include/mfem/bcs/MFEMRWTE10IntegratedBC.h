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

  // Utilities
  mfem::Vector CrossProduct(const mfem::Vector & va, const mfem::Vector & vb)
  {
    mfem::Vector vec;
    vec.SetSize(3);
    vec[0] = va[1] * vb[2] - va[2] * vb[1];
    vec[1] = va[2] * vb[0] - va[0] * vb[2];
    vec[2] = va[0] * vb[1] - va[1] * vb[0];
    return vec;
  }

  // Utilities
  mfem::real_t InnerProduct(const mfem::Vector & va, const mfem::Vector & vb)
  {
    if (va.Size() != vb.Size())
      mooseError("InnerProduct: vectors must have the same size");

    mfem::real_t res = 0.0;
    for (int i = 0; i < va.Size(); ++i)
      res += va[i] * vb[i];

    return res;
  }

  static inline mfem::Vector to3DMFEMVector(const RealVectorValue & v)
  {
    mfem::Vector mfem_vec(3);
    for (int i = 0; i < 3; ++i)
      mfem_vec(i) = v(i);
    return mfem_vec;
  }

protected:
  mfem::real_t _mu;
  mfem::real_t _epsilon;

  mfem::real_t _omega;
  mfem::Vector _a1_vec;
  mfem::Vector _a2_vec;
  mfem::Vector _a3_vec;
  mfem::Vector _a2xa3;
  mfem::Vector _a3xa1;

  mfem::real_t _v;
  mfem::real_t _kc;
  mfem::real_t _k0;
  std::complex<mfem::real_t> _k;

  mfem::Vector _k_a;
  mfem::Vector _k_c;

  std::unique_ptr<mfem::ConstantCoefficient> _robin_coef_im{nullptr};
  std::unique_ptr<mfem::VectorFunctionCoefficient> _u_real{nullptr};
  std::unique_ptr<mfem::VectorFunctionCoefficient> _u_imag{nullptr};
};

#endif
