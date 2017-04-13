/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputePlaneIncrementalStrain.h"

template <>
InputParameters
validParams<ComputePlaneIncrementalStrain>()
{
  InputParameters params = validParams<Compute2DIncrementalStrain>();
  params.addClassDescription(
      "Compute strain increment for small strain under 2D planar assumptions.");
  params.addCoupledVar("scalar_out_of_plane_strain",
                       "Scalar variable for generalized plane strain");
  params.addCoupledVar("out_of_plane_strain", "Nonlinear variable for plane stress condition");

  return params;
}

ComputePlaneIncrementalStrain::ComputePlaneIncrementalStrain(const InputParameters & parameters)
  : Compute2DIncrementalStrain(parameters),
    _scalar_out_of_plane_strain_coupled(isCoupledScalar("scalar_out_of_plane_strain")),
    _scalar_out_of_plane_strain(_scalar_out_of_plane_strain_coupled
                                    ? coupledScalarValue("scalar_out_of_plane_strain")
                                    : _zero),
    _scalar_out_of_plane_strain_old(_scalar_out_of_plane_strain_coupled
                                        ? coupledScalarValueOld("scalar_out_of_plane_strain")
                                        : _zero),
    _out_of_plane_strain_coupled(isCoupled("out_of_plane_strain")),
    _out_of_plane_strain(_out_of_plane_strain_coupled ? coupledValue("out_of_plane_strain")
                                                      : _zero),
    _out_of_plane_strain_old(_out_of_plane_strain_coupled ? coupledValueOld("out_of_plane_strain")
                                                          : _zero)
{
  if (_out_of_plane_strain_coupled && _scalar_out_of_plane_strain_coupled)
    mooseError("Must define only one of out_of_plane_strain or scalar_out_of_plane_strain");
}

Real
ComputePlaneIncrementalStrain::computeGradDispZZ()
{
  if (_scalar_out_of_plane_strain_coupled)
    return _scalar_out_of_plane_strain[0];
  else
    return _out_of_plane_strain[_qp];
}

Real
ComputePlaneIncrementalStrain::computeGradDispZZOld()
{
  if (_scalar_out_of_plane_strain_coupled)
    return _scalar_out_of_plane_strain_old[0];
  else
    return _out_of_plane_strain_old[_qp];
}
