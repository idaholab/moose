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

class MFEMComplexIntegratedBC : public MFEMIntegratedBC
{
public:
  static InputParameters validParams();

  MFEMComplexIntegratedBC(const InputParameters & parameters);
  virtual ~MFEMComplexIntegratedBC() = default;

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

  virtual void setRealBC(std::shared_ptr<MFEMIntegratedBC> bc) { _real_bc = bc; }
  virtual void setImagBC(std::shared_ptr<MFEMIntegratedBC> bc) { _imag_bc = bc; }

  virtual mfem::LinearFormIntegrator * createLFIntegrator() override
  {
    mooseError("MFEMComplexIntegratedBC does not support createLFIntegrator(). Please use "
               "getRealLFIntegrator() and getImagLFIntegrator()");
  }
  virtual mfem::BilinearFormIntegrator * createBFIntegrator() override
  {
    mooseError("MFEMComplexIntegratedBC does not support createBFIntegrator(). Please use "
               "getRealBFIntegrator() and getImagBFIntegrator()");
  }

  // Get name of the trial variable (gridfunction) the kernel acts on.
  // Defaults to the name of the test variable labelling the weak form.
  virtual const std::string & getTrialVariableName() const { return _test_var_name; }

protected:
  std::shared_ptr<MFEMIntegratedBC> _real_bc{nullptr};
  std::shared_ptr<MFEMIntegratedBC> _imag_bc{nullptr};
};

#endif
