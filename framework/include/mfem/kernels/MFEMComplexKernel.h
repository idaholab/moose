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

#include "MFEMGeneralUserObject.h"
#include "MFEMContainers.h"
#include "MFEMBlockRestrictable.h"
#include "MFEMKernel.h"

/*
Class to construct an MFEM integrator to apply to the equation system.
*/
class MFEMComplexKernel : public MFEMGeneralUserObject, public MFEMBlockRestrictable
{
public:
  static InputParameters validParams();

  MFEMComplexKernel(const InputParameters & parameters);

  virtual ~MFEMComplexKernel() = default;

  virtual mfem::LinearFormIntegrator * getRealLFIntegrator() { return _real_kernel->createLFIntegrator(); }
  virtual mfem::LinearFormIntegrator * getImagLFIntegrator() { return _imag_kernel->createLFIntegrator(); }
  virtual mfem::BilinearFormIntegrator * getRealBFIntegrator() { return _real_kernel->createBFIntegrator();}
  virtual mfem::BilinearFormIntegrator * getImagBFIntegrator() { return _imag_kernel->createBFIntegrator(); }

  virtual void setRealKernel(std::shared_ptr<MFEMKernel> kernel) { _real_kernel = kernel; }
  virtual void setImagKernel(std::shared_ptr<MFEMKernel> kernel) { _imag_kernel = kernel; }

  // Get name of the test variable labelling the weak form this kernel is added to
  const VariableName & getTestVariableName() const { return _test_var_name; }

  // Get name of the trial variable (gridfunction) the kernel acts on.
  // Defaults to the name of the test variable labelling the weak form.
  virtual const VariableName & getTrialVariableName() const { return _test_var_name; }

protected:
  // Name of (the test variable associated with) the weak form that the kernel is applied to.
  const VariableName & _test_var_name;
  std::shared_ptr<MFEMKernel> _real_kernel{nullptr};
  std::shared_ptr<MFEMKernel> _imag_kernel{nullptr};  

};

#endif
