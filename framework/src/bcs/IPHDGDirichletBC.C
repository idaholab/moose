//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IPHDGDirichletBC.h"
#include "IPHDGAssemblyHelper.h"

InputParameters
IPHDGDirichletBC::validParams()
{
  auto params = IPHDGBC::validParams();
  params.addRequiredParam<MooseFunctorName>("functor",
                                            "The Dirichlet value for the primal variable");
  return params;
}

IPHDGDirichletBC::IPHDGDirichletBC(const InputParameters & params)
  : IPHDGBC(params), _dirichlet_val(getFunctor<Real>("functor"))
{
}

void
IPHDGDirichletBC::compute()
{
  auto & iphdg_helper = iphdgHelper();
  iphdg_helper.resizeResiduals();

  // u, lm_u
  iphdg_helper.scalarDirichlet(_dirichlet_val);
  iphdg_helper.lmDirichlet(_dirichlet_val);
}
