//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "NLScaleIntegrator.h"

namespace Moose::MFEM
{

void
NLScaleIntegrator::AssembleElementVector(const mfem::FiniteElement & el,
                                         mfem::ElementTransformation & Tr,
                                         const mfem::Vector & elfun,
                                         mfem::Vector & elvect)
{
  CheckIntegrator();
  _integrator->AssembleElementVector(el, Tr, elfun, elvect);
  elvect *= _scale;
}

void
NLScaleIntegrator::AssembleFaceVector(const mfem::FiniteElement & el1,
                                      const mfem::FiniteElement & el2,
                                      mfem::FaceElementTransformations & Tr,
                                      const mfem::Vector & elfun,
                                      mfem::Vector & elvect)
{
  CheckIntegrator();
  _integrator->AssembleFaceVector(el1, el2, Tr, elfun, elvect);
  elvect *= _scale;
}

void
NLScaleIntegrator::AssembleElementGrad(const mfem::FiniteElement & el,
                                       mfem::ElementTransformation & Tr,
                                       const mfem::Vector & elfun,
                                       mfem::DenseMatrix & elmat)
{
  CheckIntegrator();
  _integrator->AssembleElementGrad(el, Tr, elfun, elmat);
  elmat *= _scale;
}

void
NLScaleIntegrator::AssembleFaceGrad(const mfem::FiniteElement & el1,
                                    const mfem::FiniteElement & el2,
                                    mfem::FaceElementTransformations & Tr,
                                    const mfem::Vector & elfun,
                                    mfem::DenseMatrix & elmat)
{
  CheckIntegrator();
  _integrator->AssembleFaceGrad(el1, el2, Tr, elfun, elmat);
  elmat *= _scale;
}

mfem::real_t
NLScaleIntegrator::GetElementEnergy(const mfem::FiniteElement & el,
                                    mfem::ElementTransformation & Tr,
                                    const mfem::Vector & elfun)
{
  CheckIntegrator();
  return _scale * _integrator->GetElementEnergy(el, Tr, elfun);
}

} // namespace Moose::MFEM

#endif
