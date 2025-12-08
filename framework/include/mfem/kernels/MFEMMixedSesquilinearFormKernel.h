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
#include "MFEMMixedBilinearFormKernel.h"
#include "MFEMComplexKernel.h"

/**
 * Class to construct an MFEM mixed sesquilinear form integrator to apply to the equation system.
 */
class MFEMMixedSesquilinearFormKernel : public MFEMComplexKernel
{
public:
  static InputParameters validParams();

  MFEMMixedSesquilinearFormKernel(const InputParameters & parameters);
  ~MFEMMixedSesquilinearFormKernel() = default;

  /// Get name of the trial variable (gridfunction) the kernel acts on.
  /// Defaults to the name of the test variable labelling the weak form.
  virtual const VariableName & getTrialVariableName() const override;

  virtual mfem::LinearFormIntegrator * getRealLFIntegrator() override {return nullptr;}
  virtual mfem::LinearFormIntegrator * getImagLFIntegrator() override {return nullptr;}

  virtual mfem::BilinearFormIntegrator * getRealBFIntegrator() override
  {
    if (_real_kernel)
      return _transpose ? new mfem::TransposeIntegrator(_real_kernel->createMBFIntegrator()) : _real_kernel->createMBFIntegrator();
    else
      return nullptr;
  }

  virtual mfem::BilinearFormIntegrator * getImagBFIntegrator() override
  {
    if (_imag_kernel)
      return _transpose ? new mfem::TransposeIntegrator(_imag_kernel->createMBFIntegrator()) : _imag_kernel->createMBFIntegrator();
    else
      return nullptr;
  }

protected:
  /// Name of the trial variable that the kernel is applied to.
  const VariableName _trial_var_name;
  /// Bool controlling whether to add the transpose of the integrator to the system
  bool _transpose;

  std::shared_ptr<MFEMMixedBilinearFormKernel> _real_kernel{nullptr};
  std::shared_ptr<MFEMMixedBilinearFormKernel> _imag_kernel{nullptr};

};

#endif
