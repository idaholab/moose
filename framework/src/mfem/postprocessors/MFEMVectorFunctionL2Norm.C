//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorFunctionL2Norm.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMVectorFunctionL2Norm);

InputParameters
MFEMVectorFunctionL2Norm::validParams()
{
  InputParameters params = MFEMNormPostprocessor::validParams();
  params.addClassDescription(
      "Computes the L2 norm $\\left\\Vert \\vec f\\right\\Vert_{\\rm L2}$ of a vector function "
      "over the mesh.");
  params.addRequiredParam<MFEMVectorCoefficientName>("function",
                                                     "The function of which to find the norm.");
  // Without a variable, there is no element order to choose a default quadrature rule from
  params.makeParamRequired<unsigned int>("quadrature_order");
  params.setDocString("quadrature_order", "Order of the quadrature rule used on every element.");
  return params;
}

MFEMVectorFunctionL2Norm::MFEMVectorFunctionL2Norm(const InputParameters & parameters)
  : MFEMNormPostprocessor(parameters),
    _vec_coeff(getVectorCoefficient("function")),
    _mesh(getMFEMProblem().mesh())
{
}

PostprocessorValue
MFEMVectorFunctionL2Norm::getValue() const
{
  return mfem::ComputeGlobalLpNorm(2.0, _vec_coeff, _mesh.getMFEMParMesh(), getIntegrationRules());
}

#endif
