//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#pragma once
#include "MFEMIntegratedBC.h"

class MFEMComplexIntegratedBC : public MFEMBoundaryCondition
{
public:
  static InputParameters validParams();

  MFEMComplexIntegratedBC(const InputParameters & parameters);
  virtual ~MFEMComplexIntegratedBC() = default;

  virtual mfem::LinearFormIntegrator * getRealLFIntegrator() { return _real_bc->createLFIntegrator(); }
  virtual mfem::LinearFormIntegrator * getImagLFIntegrator() { return _imag_bc->createLFIntegrator(); }
  virtual mfem::BilinearFormIntegrator * getRealBFIntegrator() { return _real_bc->createBFIntegrator();}
  virtual mfem::BilinearFormIntegrator * getImagBFIntegrator() { return _imag_bc->createBFIntegrator(); }

  virtual void setRealBC(std::shared_ptr<MFEMIntegratedBC> bc) { _real_bc = bc; }
  virtual void setImagBC(std::shared_ptr<MFEMIntegratedBC> bc) { _imag_bc = bc; }

  // Get name of the trial variable (gridfunction) the kernel acts on.
  // Defaults to the name of the test variable labelling the weak form.
  virtual const std::string & getTrialVariableName() const { return _test_var_name; }

protected:

  std::shared_ptr<MFEMIntegratedBC> _real_bc{nullptr};
  std::shared_ptr<MFEMIntegratedBC> _imag_bc{nullptr};  

};

#endif
