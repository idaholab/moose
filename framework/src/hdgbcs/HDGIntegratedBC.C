//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HDGIntegratedBC.h"
#include "AuxiliarySystem.h"

InputParameters
HDGIntegratedBC::validParams()
{
  InputParameters params = IntegratedBCBase::validParams();
  params.suppressParameter<std::vector<AuxVariableName>>("save_in");
  params.suppressParameter<std::vector<AuxVariableName>>("diag_save_in");
  params.registerBase("HDGIntegratedBC");
  return params;
}

HDGIntegratedBC::HDGIntegratedBC(const InputParameters & parameters)
  : IntegratedBCBase(parameters),
    ADFunctorInterface(this),
    _normals(_assembly.normals()),
    _aux_sys(_fe_problem.getAuxiliarySystem()),
    _JxW_face(_JxW),
    _qrule_face(_qrule),
    _q_point_face(_q_point)
{
}
