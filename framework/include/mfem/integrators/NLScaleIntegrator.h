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

#include "MFEMGeneralUserObject.h"
#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/pfem_extras.hpp"
#include "libmesh/restore_warnings.h"

namespace Moose::MFEM
{

/// NonlinearFormIntegrator which scales its results by a constant value
class NLScaleIntegrator : public mfem::NonlinearFormIntegrator
{
private:
  mfem::NonlinearFormIntegrator * _integrator{nullptr};
  mfem::real_t _scale;
  bool _own_integrator;

public:
  NLScaleIntegrator(mfem::NonlinearFormIntegrator * integ)
    : _integrator{integ}, _scale{1}, _own_integrator{true}
  {
  }
  NLScaleIntegrator(mfem::NonlinearFormIntegrator * integ, mfem::real_t scale)
    : _integrator{integ}, _scale{scale}, _own_integrator{true}
  {
  }
  NLScaleIntegrator(mfem::NonlinearFormIntegrator * integ, mfem::real_t scale, bool own)
    : _integrator{integ}, _scale{scale}, _own_integrator{own}
  {
  }

  void SetIntegrator(mfem::NonlinearFormIntegrator * integ)
  {
    if (_integrator && _own_integrator)
    {
      delete _integrator;
    }

    _integrator = integ;
  }

  void SetScale(mfem::real_t scale) { _scale = scale; }

  void SetOwn(bool own) { _own_integrator = own; }

  void CheckIntegrator()
  {
    if (!_integrator)
      mooseError("Integrator not set");
  }

  void SetIntegrationMode(mfem::NonlinearFormIntegrator::Mode m)
  {
    integrationMode = m;
    _integrator->SetIntegrationMode(m);
  }

  bool Patchwise() const { return integrationMode != Mode::ELEMENTWISE; }

  /// Set the memory type used for GeometricFactors and other large allocations
  /// in PA extensions.
  void SetPAMemoryType(mfem::MemoryType mt)
  {
    pa_mt = mt;
    _integrator->SetPAMemoryType(mt);
  }

  virtual void AssembleElementVector(const mfem::FiniteElement & el,
                                     mfem::ElementTransformation & Tr,
                                     const mfem::Vector & elfun,
                                     mfem::Vector & elvect) override;

  virtual void AssembleFaceVector(const mfem::FiniteElement & el1,
                                  const mfem::FiniteElement & el2,
                                  mfem::FaceElementTransformations & Tr,
                                  const mfem::Vector & elfun,
                                  mfem::Vector & elvect) override;

  virtual void AssembleElementGrad(const mfem::FiniteElement & el,
                                   mfem::ElementTransformation & Tr,
                                   const mfem::Vector & elfun,
                                   mfem::DenseMatrix & elmat) override;

  virtual void AssembleFaceGrad(const mfem::FiniteElement & el1,
                                const mfem::FiniteElement & el2,
                                mfem::FaceElementTransformations & Tr,
                                const mfem::Vector & elfun,
                                mfem::DenseMatrix & elmat) override;

  virtual mfem::real_t GetElementEnergy(const mfem::FiniteElement & el,
                                        mfem::ElementTransformation & Tr,
                                        const mfem::Vector & elfun) override;

  virtual ~NLScaleIntegrator()
  {
    if (_own_integrator)
      delete _integrator;
  };
};
} // namespace Moose::MFEM

#endif
