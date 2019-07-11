//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADMatchedScalarValueBC.h"
#include "Function.h"

#include "libmesh/node.h"

registerADMooseObject("MooseTestApp", ADMatchedScalarValueBC);

defineADValidParams(ADMatchedScalarValueBC,
                    ADNodalBC,
                    params.addRequiredCoupledVar("v", "The scalar variable to match"););

template <ComputeStage compute_stage>
ADMatchedScalarValueBC<compute_stage>::ADMatchedScalarValueBC(const InputParameters & parameters)
  : ADNodalBC<compute_stage>(parameters), _v(adCoupledScalarValue("v"))
{
}

template <ComputeStage compute_stage>
ADReal
ADMatchedScalarValueBC<compute_stage>::computeQpResidual()
{
  return _u - _v[0];
}
