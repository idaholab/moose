/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeAxisymmetric1DIncrementalStrain.h"
#include "Assembly.h"

template <>
InputParameters
validParams<ComputeAxisymmetric1DIncrementalStrain>()
{
  InputParameters params = validParams<Compute1DIncrementalStrain>();
  params.addClassDescription(
      "Compute strain increment for small strains in an axisymmetric 1D problem");
  params.addCoupledVar("scalar_out_of_plane_strain", "Scalar variable for axisymmetric 1D problem");
  params.addCoupledVar("out_of_plane_strain", "Nonlinear variable for axisymmetric 1D problem");

  return params;
}

ComputeAxisymmetric1DIncrementalStrain::ComputeAxisymmetric1DIncrementalStrain(
    const InputParameters & parameters)
  : Compute1DIncrementalStrain(parameters),
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
ComputeAxisymmetric1DIncrementalStrain::initialSetup()
{
  if (getBlockCoordSystem() != Moose::COORD_RZ)
    mooseError("The coordinate system must be set to RZ for Axisymmetric 1D simulations");
}

Real
ComputeAxisymmetric1DIncrementalStrain::computeGradDispYY()
{
  if (_scalar_out_of_plane_strain_coupled)
    return _scalar_out_of_plane_strain[0];
  else
    return _out_of_plane_strain[_qp];
}

Real
ComputeAxisymmetric1DIncrementalStrain::computeGradDispYYOld()
{
  if (_scalar_out_of_plane_strain_coupled)
    return _scalar_out_of_plane_strain_old[0];
  else
    return _out_of_plane_strain_old[_qp];
}

Real
ComputeAxisymmetric1DIncrementalStrain::computeGradDispZZ()
{
  if (!MooseUtils::absoluteFuzzyEqual(_q_point[_qp](0), 0.0))
    return (*_disp[0])[_qp] / _q_point[_qp](0);
  else
    return 0.0;
}

Real
ComputeAxisymmetric1DIncrementalStrain::computeGradDispZZOld()
{
  if (!MooseUtils::absoluteFuzzyEqual(_q_point[_qp](0), 0.0))
    return _disp_old_0[_qp] / _q_point[_qp](0);
  else
    return 0.0;
}
