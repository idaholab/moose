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

registerMooseMFEMObject("MooseApp", ComplexKernel);

namespace Moose::MFEM
{
InputParameters
ComplexKernel::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription(
      "Holds Moose::MFEM::Kernel objects for the real and imaginary parts of a complex kernel.");

  return params;
}

ComplexKernel::ComplexKernel(const InputParameters & parameters) : Kernel(parameters) {}

} // namespace Moose::MFEM
#endif
