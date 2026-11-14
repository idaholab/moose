//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarContactPressureIntegral.h"
#include "MortarContactPressureProvider.h"
#include "Assembly.h"
#include "libmesh/quadrature.h"

registerMooseObject("MooseTestApp", MortarContactPressureIntegral);

InputParameters
MortarContactPressureIntegral::validParams()
{
  InputParameters params = MortarUserObject::validParams();
  params.addClassDescription(
      "Integrates the contact pressure supplied by a mortar contact pressure "
      "provider over a mortar interface.");
  params.addRequiredParam<UserObjectName>("contact_pressure_uo",
                                          "The object that provides the contact pressure.");
  return params;
}

MortarContactPressureIntegral::MortarContactPressureIntegral(const InputParameters & parameters)
  : MortarUserObject(parameters),
    _provider(getUserObject<MortarContactPressureProvider>("contact_pressure_uo")),
    _coord(_assembly.mortarCoordTransformation())
{
  _provider.checkConsumer(*this,
                          "contact_pressure_uo",
                          primarySubdomain(),
                          secondarySubdomain(),
                          getParam<bool>("use_displaced_mesh"));
}

void
MortarContactPressureIntegral::execute()
{
  for (const auto qp : make_range(_qrule_msm->n_points()))
    _integral += _JxW_msm[qp] * _coord[qp] *
                 MetaPhysicL::raw_value(_provider.qpContactPressure(*_lower_secondary_elem, qp));
}

void
MortarContactPressureIntegral::finalize()
{
  gatherSum(_integral);
}
