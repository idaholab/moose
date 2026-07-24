//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdvectionLHDGAssemblyHelper.h"
#include "AdvectionLHDGDirichletBC.h"

registerMooseObject("NavierStokesApp", AdvectionLHDGDirichletBC);

InputParameters
AdvectionLHDGDirichletBC::validParams()
{
  auto params = TwoFieldScalarHDGBC::validParams();
  params += AdvectionLHDGAssemblyHelper::validParams();
  params.addRequiredParam<MooseFunctorName>("functor", "The prescribed scalar value");
  params.addClassDescription("Weakly imposes prescribed scalar and velocity data for an L-HDG "
                             "advection discretization.");
  return params;
}

AdvectionLHDGDirichletBC::AdvectionLHDGDirichletBC(const InputParameters & parameters)
  : TwoFieldScalarHDGBC(parameters),
    _lhdg_helper(std::make_unique<AdvectionLHDGAssemblyHelper>(
        this, this, this, _sys, _assembly, _tid, std::set<SubdomainID>{}, boundaryIDs())),
    _dirichlet_value(getFunctor<Real>("functor"))
{
}

void
AdvectionLHDGDirichletBC::compute(TwoFieldScalarHDGAssemblyHelper &)
{
  _lhdg_helper->resizeResiduals();
  _lhdg_helper->scalarDirichlet(_dirichlet_value);
  // L-HDG Dirichlet traces are unused and conventionally constrained to zero.
  _lhdg_helper->lmDirichletZero();
}

TwoFieldScalarHDGAssemblyHelper &
AdvectionLHDGDirichletBC::hdgHelper()
{
  return *_lhdg_helper;
}
