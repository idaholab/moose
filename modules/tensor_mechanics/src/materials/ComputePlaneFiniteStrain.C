/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputePlaneFiniteStrain.h"

template <>
InputParameters
validParams<ComputePlaneFiniteStrain>()
{
  InputParameters params = validParams<Compute2DFiniteStrain>();
  params.addClassDescription("Compute strain increment and rotation increment for finite strain "
                             "under 2D planar assumptions.");
  params.addCoupledVar("scalar_out_of_plane_strain",
                       "Scalar variable for generalized plane strain");
  params.addCoupledVar("out_of_plane_strain", "Nonlinear variable for plane stress condition");

  return params;
}

ComputePlaneFiniteStrain::ComputePlaneFiniteStrain(const InputParameters & parameters)
  : Compute2DFiniteStrain(parameters),
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
ComputePlaneFiniteStrain::computeGradDispZZ()
{
  /**
   * This is consistent with the approximation of stretch rate tensor
   * D = log(sqrt(Fhat^T * Fhat)) / dt
   */
  if (_scalar_out_of_plane_strain_coupled)
    return std::exp(_scalar_out_of_plane_strain[0]) - 1.0;
  else
    return std::exp(_out_of_plane_strain[_qp]) - 1.0;
}

Real
ComputePlaneFiniteStrain::computeGradDispZZOld()
{
  if (_scalar_out_of_plane_strain_coupled)
    return std::exp(_scalar_out_of_plane_strain_old[0]) - 1.0;
  else
    return std::exp(_out_of_plane_strain_old[_qp]) - 1.0;
}
