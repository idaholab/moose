//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMDGKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMDGKernel);

InputParameters
MFEMDGKernel::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params += MFEMBlockRestrictable::validParams();
  params.registerBase("DGKernel");
  params.addParam<VariableName>("variable",
                                "Variable labelling the weak form this kernel is added to");
  params.addParam<mfem::real_t>("sigma", -1.0, "One of the DG penalty params. Typically +/- 1.0");
  params.addParam<mfem::real_t>(
      "kappa", "One of the DG penalty params. Should be positive. Will default to (order+1)^2");
  return params;
}

MFEMDGKernel::MFEMDGKernel(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters),
    MFEMBlockRestrictable(parameters,
                          getMFEMProblem().getMFEMVariableMesh(getParam<VariableName>("variable"))),
    _test_var_name(getParam<VariableName>("variable")),
    _fe_order(getMFEMProblem()
                  .getProblemData()
                  .gridfunctions.Get(_test_var_name)
                  ->ParFESpace()
                  ->FEColl()
                  ->GetOrder()),
    _one(1.0),
    _zero(0.0),
    _sigma(getParam<mfem::real_t>("sigma")),
    _kappa((isParamSetByUser("kappa")) ? getParam<mfem::real_t>("kappa")
                                       : (_fe_order + 1) * (_fe_order + 1))
{
}

#endif
