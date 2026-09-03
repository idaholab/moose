//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMConstraint.h"
#include "MFEMProblem.h"

InputParameters
MFEMConstraint::validParams()
{
  InputParameters params = MFEMObject::validParams();
  params += MFEMBlockRestrictable::validParams();

  params.addClassDescription("Base class for constraining an MFEM variable within one or more mesh "
                             "subdomains.");
  params.registerBase("Constraint");
  params.registerSystemAttributeName("Constraint");
  params.addParam<VariableName>("variable", "Variable on which to apply the constraint");
  return params;
}

MFEMConstraint::MFEMConstraint(const InputParameters & parameters)
  : MFEMObject(parameters),
    MFEMBlockRestrictable(parameters,
                          getMFEMProblem().getMFEMVariableMesh(getParam<VariableName>("variable"))),
    _trial_var_name(getParam<VariableName>("variable"))
{
}

#endif
