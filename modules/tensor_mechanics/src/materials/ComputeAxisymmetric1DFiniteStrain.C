/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputeAxisymmetric1DFiniteStrain.h"
#include "Assembly.h"
#include "FEProblem.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<ComputeAxisymmetric1DFiniteStrain>()
{
  InputParameters params = validParams<Compute1DFiniteStrain>();
  params.addClassDescription("Compute a strain increment and rotation increment for finite strains "
                             "in an axisymmetric 1D problem");
  params.addCoupledVar("scalar_out_of_plane_strain", "Scalar variable for axisymmetric 1D problem");
  params.addCoupledVar("out_of_plane_strain", "Nonlinear variable for axisymmetric 1D problem");

  return params;
}

ComputeAxisymmetric1DFiniteStrain::ComputeAxisymmetric1DFiniteStrain(
    const InputParameters & parameters)
  : Compute1DFiniteStrain(parameters),
    _disp_old_0(coupledValueOld("displacements", 0)),
    _out_of_plane_strain_coupled(isCoupled("out_of_plane_strain")),
    _out_of_plane_strain(_out_of_plane_strain_coupled ? coupledValue("out_of_plane_strain")
                                                      : _zero),
    _out_of_plane_strain_old(_out_of_plane_strain_coupled ? coupledValueOld("out_of_plane_strain")
                                                          : _zero),
    _scalar_out_of_plane_strain_coupled(isCoupledScalar("scalar_out_of_plane_strain")),
    _scalar_out_of_plane_strain(_scalar_out_of_plane_strain_coupled
                                    ? coupledScalarValue("scalar_out_of_plane_strain")
                                    : _zero),
    _scalar_out_of_plane_strain_old(_scalar_out_of_plane_strain_coupled
                                        ? coupledScalarValueOld("scalar_out_of_plane_strain")
                                        : _zero)
{
  if (_out_of_plane_strain_coupled && _scalar_out_of_plane_strain_coupled)
    mooseError("Must define only one of out_of_plane_strain or scalar_out_of_plane_strain");
}

void
ComputeAxisymmetric1DFiniteStrain::initialSetup()
{
  if (getBlockCoordSystem() != Moose::COORD_RZ)
    mooseError("The coordinate system must be set to RZ for Axisymmetric geometries.");
}

Real
ComputeAxisymmetric1DFiniteStrain::computeGradDispYY()
{
  if (_scalar_out_of_plane_strain_coupled)
    return std::exp(_scalar_out_of_plane_strain[0]) - 1.0;
  else
    return std::exp(_out_of_plane_strain[_qp]) - 1.0;
}

Real
ComputeAxisymmetric1DFiniteStrain::computeGradDispYYOld()
{
  if (_scalar_out_of_plane_strain_coupled)
    return std::exp(_scalar_out_of_plane_strain_old[0]) - 1.0;
  else
    return std::exp(_out_of_plane_strain_old[_qp]) - 1.0;
}

Real
ComputeAxisymmetric1DFiniteStrain::computeGradDispZZ()
{
  if (!MooseUtils::absoluteFuzzyEqual(_q_point[_qp](0), 0.0))
    return (*_disp[0])[_qp] / _q_point[_qp](0);
  else
    return 0.0;
}

Real
ComputeAxisymmetric1DFiniteStrain::computeGradDispZZOld()
{
  if (!MooseUtils::absoluteFuzzyEqual(_q_point[_qp](0), 0.0))
    return _disp_old_0[_qp] / _q_point[_qp](0);
  else
    return 0.0;
}
