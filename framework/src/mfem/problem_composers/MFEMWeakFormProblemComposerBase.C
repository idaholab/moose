//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMWeakFormProblemComposerBase.h"

InputParameters
MFEMWeakFormProblemComposerBase::validParams()
{
  InputParameters params = MFEMProblemComposer::validParams();
  params.addParam<MFEMWeakFormName>(
      "weak_form",
      "Name of the weak form in the WeakForms block whose equation system this operator solves. "
      "May be omitted only if the problem has a single weak form, whose equation system is then "
      "used.");
  return params;
}

MFEMWeakFormProblemComposerBase::MFEMWeakFormProblemComposerBase(const InputParameters & parameters)
  : MFEMProblemComposer(parameters),
    _weak_form_name(isParamValid("weak_form") ? getParam<MFEMWeakFormName>("weak_form")
                                              : MFEMWeakFormName())
{
}

#endif
