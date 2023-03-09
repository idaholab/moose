//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPermeabilityConstFromVar.h"

registerMooseObject("PorousFlowApp", PorousFlowPermeabilityConstFromVar);

InputParameters
PorousFlowPermeabilityConstFromVar::validParams()
{
  InputParameters params = PorousFlowPermeabilityBase::validParams();
  params.addRequiredCoupledVar("perm_xx", "The xx component of the permeability tensor");
  params.addRequiredCoupledVar("perm_yy", "The yy component of the permeability tensor");
  params.addRequiredCoupledVar("perm_zz", "The zz component of the permeability tensor");
  params.addCoupledVar("perm_xy", 0.0, "The xy component of the permeability tensor");
  params.addCoupledVar("perm_xz", 0.0, "The xz component of the permeability tensor");
  params.addCoupledVar("perm_yx", 0.0, "The yx component of the permeability tensor");
  params.addCoupledVar("perm_yz", 0.0, "The yz component of the permeability tensor");
  params.addCoupledVar("perm_zx", 0.0, "The zx component of the permeability tensor");
  params.addCoupledVar("perm_zy", 0.0, "The zy component of the permeability tensor");
  params.addClassDescription(
      "This Material calculates the permeability tensor given by the input variables");
  return params;
}

PorousFlowPermeabilityConstFromVar::PorousFlowPermeabilityConstFromVar(
    const InputParameters & parameters)
  : PorousFlowPermeabilityBase(parameters),
    _perm_xx(coupledValue("perm_xx")),
    _perm_xy(coupledValue("perm_xy")),
    _perm_xz(coupledValue("perm_xz")),
    _perm_yx(coupledValue("perm_yx")),
    _perm_yy(coupledValue("perm_yy")),
    _perm_yz(coupledValue("perm_yz")),
    _perm_zx(coupledValue("perm_zx")),
    _perm_zy(coupledValue("perm_zy")),
    _perm_zz(coupledValue("perm_zz"))
{
}

void
PorousFlowPermeabilityConstFromVar::computeQpProperties()
{
  RealTensorValue permeability(_perm_xx[_qp],
                               _perm_xy[_qp],
                               _perm_xz[_qp],
                               _perm_yx[_qp],
                               _perm_yy[_qp],
                               _perm_yz[_qp],
                               _perm_zx[_qp],
                               _perm_zy[_qp],
                               _perm_zz[_qp]);

  _permeability_qp[_qp] = permeability;
  (*_dpermeability_qp_dvar)[_qp].resize(_num_var, RealTensorValue());
  (*_dpermeability_qp_dgradvar)[_qp].resize(LIBMESH_DIM);

  for (const auto i : make_range(Moose::dim))
    (*_dpermeability_qp_dgradvar)[_qp][i].resize(_num_var, RealTensorValue());
}
