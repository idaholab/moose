//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeWeightedGapLMMechanicalContact.h"
#include "SubProblem.h"

registerMooseObject("ContactApp", ComputeWeightedGapLMMechanicalContact);

InputParameters
ComputeWeightedGapLMMechanicalContact::validParams()
{
  InputParameters params = ADMortarConstraint::validParams();
  params.addClassDescription("Computes the weighted gap that will later be used to enforce the "
                             "zero-penetration mechanical contact conditions");
  params.addRequiredCoupledVar("secondary_disp_y",
                               "The y displacement variable on the secondary face");
  params.addCoupledVar("primary_disp_y",
                       "The y displacement variable on the primary face. If this is not provided, "
                       "then the secondary y-displacement variable will be used");
  return params;
}

ComputeWeightedGapLMMechanicalContact::ComputeWeightedGapLMMechanicalContact(
    const InputParameters & parameters)
  : ADMortarConstraint(parameters),
    _secondary_disp_y(adCoupledValue("secondary_disp_y")),
    _primary_disp_y(isCoupled("primary_disp_y") ? adCoupledNeighborValue("primary_disp_y")
                                                : adCoupledNeighborValue("secondary_disp_y"))
{
}

ADReal
ComputeWeightedGapLMMechanicalContact::computeQpResidual(Moose::MortarType mortar_type)
{
  switch (mortar_type)
  {
    case Moose::MortarType::Lower:
    {
      if (_has_primary)
      {
        DualRealVectorValue gap_vec = _phys_points_primary[_qp] - _phys_points_secondary[_qp];
        // Here we're assuming that the user provided the x-component as the secondary/primary
        // variable!
        gap_vec(0).derivatives() = _u_primary[_qp].derivatives() - _u_secondary[_qp].derivatives();
        gap_vec(1).derivatives() =
            _primary_disp_y[_qp].derivatives() - _secondary_disp_y[_qp].derivatives();

        auto gap = gap_vec * _normals[_qp];

        return _test[_i][_qp] * gap;
      }
      else
        return _test[_i][_qp] * std::numeric_limits<Real>::max();
    }

    default:
      return 0;
  }
}
