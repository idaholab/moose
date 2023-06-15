//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledVarStatsNodalReporter.h"

registerMooseObject("MooseApp", CoupledVarStatsNodalReporter);

InputParameters
CoupledVarStatsNodalReporter::validParams()
{
  InputParameters params = NodalStatsReporter::validParams();

  params.addRequiredCoupledVar("coupled_var", "Coupled variable whose value is used.");

  params.addClassDescription("Nodal reporter to get statistics for a coupled variable. This can "
                             "be transfered to other apps.");
  return params;
}

CoupledVarStatsNodalReporter::CoupledVarStatsNodalReporter(const InputParameters & parameters)
  : NodalStatsReporter(parameters), _v(coupledValue("coupled_var"))
{
}
Real
CoupledVarStatsNodalReporter::computeValue()
{
  return _v[_qp];
}
