/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputePlaneSmallStrain.h"

template <>
InputParameters
validParams<ComputePlaneSmallStrain>()
{
  InputParameters params = validParams<Compute2DSmallStrain>();
  params.addClassDescription("Compute a small strain under generalized plane strain assumptions "
                             "where the out of plane strain is generally nonzero.");
  params.addCoupledVar("scalar_out_of_plane_strain",
                       "Scalar variable for generalized plane strain");
  params.addCoupledVar("out_of_plane_strain", "Nonlinear variable for plane stress condition");

  return params;
}

ComputePlaneSmallStrain::ComputePlaneSmallStrain(const InputParameters & parameters)
  : Compute2DSmallStrain(parameters),
    _scalar_out_of_plane_strain_coupled(isCoupledScalar("scalar_out_of_plane_strain")),
    _scalar_out_of_plane_strain(_scalar_out_of_plane_strain_coupled
                                    ? coupledScalarValue("scalar_out_of_plane_strain")
                                    : _zero),
    _out_of_plane_strain_coupled(isCoupled("out_of_plane_strain")),
    _out_of_plane_strain(_out_of_plane_strain_coupled ? coupledValue("out_of_plane_strain") : _zero)
{
  if (_out_of_plane_strain_coupled && _scalar_out_of_plane_strain_coupled)
    mooseError("Must define only one of out_of_plane_strain or scalar_out_of_plane_strain");
}

Real
ComputePlaneSmallStrain::computeStrainZZ()
{
  if (_scalar_out_of_plane_strain_coupled)
    return _scalar_out_of_plane_strain[0];
  else
    return _out_of_plane_strain[_qp];
}
