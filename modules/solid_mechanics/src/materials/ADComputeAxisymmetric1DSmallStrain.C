//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeAxisymmetric1DSmallStrain.h"
#include "UserObject.h"

registerMooseObject("SolidMechanicsApp", ADComputeAxisymmetric1DSmallStrain);

InputParameters
ADComputeAxisymmetric1DSmallStrain::validParams()
{
  InputParameters params = ADCompute1DSmallStrain::validParams();
  params.addClassDescription("Compute a small strain in an Axisymmetric 1D problem");
  params.addParam<UserObjectName>("subblock_index_provider",
                                  "SubblockIndexProvider user object name");
  params.addCoupledVar("scalar_out_of_plane_strain", "Scalar variable for axisymmetric 1D problem");
  params.addCoupledVar("out_of_plane_strain", "Nonlinear variable for axisymmetric 1D problem");

  return params;
}

ADComputeAxisymmetric1DSmallStrain::ADComputeAxisymmetric1DSmallStrain(
    const InputParameters & parameters)
  : ADCompute1DSmallStrain(parameters),
    _subblock_id_provider(isParamValid("subblock_index_provider")
                              ? &getUserObject<SubblockIndexProvider>("subblock_index_provider")
                              : nullptr),
    _has_out_of_plane_strain(isCoupled("out_of_plane_strain")),
    _out_of_plane_strain(_has_out_of_plane_strain ? adCoupledValue("out_of_plane_strain")
                                                  : _ad_zero),
    _has_scalar_out_of_plane_strain(isCoupledScalar("scalar_out_of_plane_strain"))
{
  if (_has_out_of_plane_strain && _has_scalar_out_of_plane_strain)
    mooseError("Must define only one of out_of_plane_strain or scalar_out_of_plane_strain");

  if (_has_scalar_out_of_plane_strain)
  {
    const auto nscalar_strains = coupledScalarComponents("scalar_out_of_plane_strain");
    _scalar_out_of_plane_strain.resize(nscalar_strains);
    for (unsigned int i = 0; i < nscalar_strains; ++i)
      _scalar_out_of_plane_strain[i] = &adCoupledScalarValue("scalar_out_of_plane_strain", i);
  }
}

void
ADComputeAxisymmetric1DSmallStrain::initialSetup()
{
  ADComputeStrainBase::initialSetup();

  if (getBlockCoordSystem() != Moose::COORD_RZ)
    mooseError("The coordinate system must be set to RZ for Axisymmetric geometries.");
}

unsigned int
ADComputeAxisymmetric1DSmallStrain::getCurrentSubblockIndex() const
{
  return _subblock_id_provider ? _subblock_id_provider->getSubblockIndex(*_current_elem) : 0;
}

ADReal
ADComputeAxisymmetric1DSmallStrain::computeStrainYY()
{
  if (_has_scalar_out_of_plane_strain)
    return (*_scalar_out_of_plane_strain[getCurrentSubblockIndex()])[0];
  else
    return _out_of_plane_strain[_qp];
}

ADReal
ADComputeAxisymmetric1DSmallStrain::computeStrainZZ()
{
  if (!MooseUtils::absoluteFuzzyEqual(_q_point[_qp](0), 0.0))
    return (*_disp[0])[_qp] / _q_point[_qp](0);
  else
    return 0.0;
}
