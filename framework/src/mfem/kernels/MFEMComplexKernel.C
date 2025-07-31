//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMComplexKernel.h"
#include "MFEMProblem.h"
#include "ScaleIntegrator.h"

registerMooseObject("MooseApp", MFEMComplexKernel);

InputParameters
MFEMComplexKernel::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params += MFEMBlockRestrictable::validParams();
  params.registerBase("Kernel");
  params.addParam<VariableName>("variable",
                                "Variable labelling the weak form this kernel is added to");
  params.addClassDescription(
      "Holds MFEMKernel objects for the real and imaginary parts of a complex kernel.");
  params.addParam<mfem::real_t>("phase",
                                0.,
                                "Phase shift angle to apply to the bilinear form kernels, measured "
                                "in radians. Both the real and imaginary parts get multiplied by a "
                                "factor of exp(i*phase). Does not affect linear form kernels");

  return params;
}

MFEMComplexKernel::MFEMComplexKernel(const InputParameters & parameters)
  : MFEMKernel(parameters), _test_var_name(getParam<VariableName>("variable"))
{
}

mfem::BilinearFormIntegrator *
MFEMComplexKernel::getRealBFIntegrator()
{
  mfem::real_t phase = getParam<mfem::real_t>("phase");
  Moose::MFEM::ScaleIntegrator scaled_real(_real_kernel->createBFIntegrator(), cos(phase));
  Moose::MFEM::ScaleIntegrator scaled_imag(_imag_kernel->createBFIntegrator(), -sin(phase));

  mfem::SumIntegrator * shifted_integ = new mfem::SumIntegrator();
  shifted_integ->AddIntegrator(&scaled_real);
  shifted_integ->AddIntegrator(&scaled_imag);

  return shifted_integ;
}

mfem::BilinearFormIntegrator *
MFEMComplexKernel::getImagBFIntegrator()
{
  mfem::real_t phase = getParam<mfem::real_t>("phase");
  Moose::MFEM::ScaleIntegrator scaled_real(_real_kernel->createBFIntegrator(), sin(phase));
  Moose::MFEM::ScaleIntegrator scaled_imag(_imag_kernel->createBFIntegrator(), cos(phase));

  mfem::SumIntegrator * shifted_integ = new mfem::SumIntegrator();
  shifted_integ->AddIntegrator(&scaled_real);
  shifted_integ->AddIntegrator(&scaled_imag);

  return shifted_integ;
}

#endif
