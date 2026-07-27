//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef NEML2_ENABLED

// MOOSE includes
#include "NEML2SmallStrainRZ.h"

registerMooseObject("SolidMechanicsApp", NEML2SmallStrainRZ);

InputParameters
NEML2SmallStrainRZ::validParams()
{
  InputParameters params = NEML2SmallStrain::validParams();
  params.addClassDescription(
      "This user object calculates the small strain from displacement gradients in axisymmetric "
      "(RZ) coordinates. In addition to the in-plane strain components, the out-of-plane hoop "
      "strain $\\epsilon_{\\theta\\theta} = u_r / r$ is included.");
  return params;
}

NEML2SmallStrainRZ::NEML2SmallStrainRZ(const InputParameters & parameters)
  : NEML2SmallStrain(parameters), _radial_coord(_subproblem.getAxisymmetricRadialCoord())
{
  for (const auto sid : _neml2_assembly.blockIDs())
    if (_subproblem.getCoordSystem(sid) != Moose::COORD_RZ)
      paramError("assembly",
                 "NEML2SmallStrainRZ requires every assembly block to use the axisymmetric (RZ) "
                 "coordinate system.");

  const auto disp_vars = getParam<std::vector<NonlinearVariableName>>("displacements");
  if (disp_vars.size() != 2)
    paramError("displacements",
               "exactly 2 displacement variables in coordinate-axis order are required, got ",
               disp_vars.size());

  _disp_r = &_fe.getValue(disp_vars[_radial_coord]);
}

void
NEML2SmallStrainRZ::forward()
{
  // in-plane components
  NEML2SmallStrain::forward();

  // hoop strain = u_r / r; quadrature points are strictly inside elements, so r > 0
  const auto r = _neml2_assembly.qPoints().base_index({_radial_coord});
  _output.base_index_put_({2}, *_disp_r / r);
}

#endif
