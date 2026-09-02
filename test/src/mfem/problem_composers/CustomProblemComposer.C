//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "CustomProblemComposer.h"

registerMooseObject("MooseApp", CustomProblemComposer);

InputParameters
CustomProblemComposer::validParams()
{
  InputParameters params = MFEMProblemComposer::validParams();
  params.addParam<MFEMScalarCoefficientName>("coefficient","1.", "Diffusion coefficient");
  return params;
}

CustomProblemComposer::CustomProblemComposer(const InputParameters & parameters)
  : MFEMProblemComposer(parameters)
{
}

std::shared_ptr<Moose::MFEM::ProblemOperatorBase>
CustomProblemComposer::createProblemOperator(MFEMProblem & mfem_problem)
{
  return std::make_shared<CustomProblemOperator>(mfem_problem, getScalarCoefficient("coefficient") );
}

#endif
