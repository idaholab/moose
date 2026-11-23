//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMNumElements.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMNumElements);

InputParameters
MFEMNumElements::validParams()
{
  InputParameters params = MFEMPostprocessor::validParams();
  params.addClassDescription(
      "Computes the number of elements of the MFEM mesh over all processes, which changes with "
      "MFEM adaptive mesh refinement.");
  return params;
}

MFEMNumElements::MFEMNumElements(const InputParameters & parameters)
  : MFEMPostprocessor(parameters), _mesh(getMFEMProblem().mesh())
{
}

PostprocessorValue
MFEMNumElements::getValue() const
{
  return _mesh.getMFEMParMesh().GetGlobalNE();
}

#endif
