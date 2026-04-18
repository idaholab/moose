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

#include "MFEMIntegratedBC.h"

namespace Moose::MFEM
{
class ComplexIntegratedBC : public BoundaryCondition
{
public:
  static InputParameters validParams();

  ComplexIntegratedBC(const InputParameters & parameters);
  virtual ~ComplexIntegratedBC() = default;

  virtual mfem::LinearFormIntegrator * getRealLFIntegrator()
  {
    return _real_bc->createLFIntegrator();
  }
  virtual mfem::LinearFormIntegrator * getImagLFIntegrator()
  {
    return _imag_bc->createLFIntegrator();
  }
  virtual mfem::BilinearFormIntegrator * getRealBFIntegrator()
  {
    return _real_bc->createBFIntegrator();
  }
  virtual mfem::BilinearFormIntegrator * getImagBFIntegrator()
  {
    return _imag_bc->createBFIntegrator();
  }

  virtual void setRealBC(std::shared_ptr<IntegratedBC> bc) { _real_bc = bc; }
  virtual void setImagBC(std::shared_ptr<IntegratedBC> bc) { _imag_bc = bc; }

  /// Get name of the trial variable (gridfunction) the bc acts on.
  /// Defaults to the name of the test variable labelling the weak form.
  virtual const std::string & getTrialVariableName() const { return _test_var_name; }

protected:
  std::shared_ptr<IntegratedBC> _real_bc{nullptr};
  std::shared_ptr<IntegratedBC> _imag_bc{nullptr};
};

} // namespace Moose::MFEM
#endif
