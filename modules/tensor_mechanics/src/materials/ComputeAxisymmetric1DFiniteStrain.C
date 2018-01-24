//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeAxisymmetric1DFiniteStrain.h"

template <>
InputParameters
validParams<ComputeAxisymmetric1DFiniteStrain>()
{
  InputParameters params = validParams<Compute1DFiniteStrain>();
  params.addClassDescription("Compute a strain increment and rotation increment for finite strains "
                             "in an axisymmetric 1D problem");
  params.addParam<UserObjectName>("subblock_index_provider",
                                  "SubblockIndexProvider user object name");
  params.addCoupledVar("scalar_out_of_plane_strain", "Scalar variable for axisymmetric 1D problem");
  params.addCoupledVar("out_of_plane_strain", "Nonlinear variable for axisymmetric 1D problem");

  return params;
}

ComputeAxisymmetric1DFiniteStrain::ComputeAxisymmetric1DFiniteStrain(
    const InputParameters & parameters)
  : Compute1DFiniteStrain(parameters),
    _disp_old_0(coupledValueOld("displacements", 0)),
    _subblock_id_provider(isParamValid("subblock_index_provider")
                              ? &getUserObject<SubblockIndexProvider>("subblock_index_provider")
                              : nullptr),
    _has_out_of_plane_strain(isParamValid("out_of_plane_strain")),
    _out_of_plane_strain(_has_out_of_plane_strain ? coupledValue("out_of_plane_strain") : _zero),
    _out_of_plane_strain_old(_has_out_of_plane_strain ? coupledValueOld("out_of_plane_strain")
                                                      : _zero),
    _has_scalar_out_of_plane_strain(isParamValid("scalar_out_of_plane_strain")),
    _nscalar_strains(
        _has_scalar_out_of_plane_strain ? coupledScalarComponents("scalar_out_of_plane_strain") : 0)
{
  if (_has_out_of_plane_strain && _has_scalar_out_of_plane_strain)
    mooseError("Must define only one of out_of_plane_strain or scalar_out_of_plane_strain");

  if (!_has_out_of_plane_strain && !_has_scalar_out_of_plane_strain)
    mooseError("Must define either out_of_plane_strain or scalar_out_of_plane_strain");

  // in case when the provided scalar_out_of_plane_strain is not a coupled
  // scalar variable, still set _nscalar_strains = 1 but return its default value 0
  if (coupledScalarComponents("scalar_out_of_plane_strain") == 0)
    _nscalar_strains = 1;

  if (_has_scalar_out_of_plane_strain)
  {
    _scalar_out_of_plane_strain.resize(_nscalar_strains);
    _scalar_out_of_plane_strain_old.resize(_nscalar_strains);
    for (unsigned int i = 0; i < _nscalar_strains; ++i)
    {
      _scalar_out_of_plane_strain[i] = &coupledScalarValue("scalar_out_of_plane_strain", i);
      _scalar_out_of_plane_strain_old[i] = &coupledScalarValueOld("scalar_out_of_plane_strain", i);
    }
  }
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
  if (_has_scalar_out_of_plane_strain)
    return std::exp((*_scalar_out_of_plane_strain[getCurrentSubblockIndex()])[0]) - 1.0;
  else
    return std::exp(_out_of_plane_strain[_qp]) - 1.0;
}

Real
ComputeAxisymmetric1DFiniteStrain::computeGradDispYYOld()
{
  if (_has_scalar_out_of_plane_strain)
    return std::exp((*_scalar_out_of_plane_strain_old[getCurrentSubblockIndex()])[0]) - 1.0;
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
