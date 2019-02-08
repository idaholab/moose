//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeAxisymmetric1DSmallStrain.h"

registerADMooseObject("TensorMechanicsApp", ADComputeAxisymmetric1DSmallStrain);

defineADValidParams(
    ADComputeAxisymmetric1DSmallStrain,
    ADCompute1DSmallStrain,
    params.addClassDescription("Compute a small strain in an Axisymmetric 1D problem");
    params.addParam<UserObjectName>("subblock_index_provider",
                                    "SubblockIndexProvider user object name");
    params.addCoupledVar("scalar_out_of_plane_strain",
                         "Scalar variable for axisymmetric 1D problem");
    params.addCoupledVar("out_of_plane_strain", "Nonlinear variable for axisymmetric 1D problem"););

template <ComputeStage compute_stage>
ADComputeAxisymmetric1DSmallStrain<compute_stage>::ADComputeAxisymmetric1DSmallStrain(
    const InputParameters & parameters)
  : ADCompute1DSmallStrain<compute_stage>(parameters),
    _subblock_id_provider(
        isParamValid("subblock_index_provider")
            ? &this->template getUserObject<SubblockIndexProvider>("subblock_index_provider")
            : nullptr),
    _has_out_of_plane_strain(isParamValid("out_of_plane_strain")),
    _out_of_plane_strain(_has_out_of_plane_strain ? adCoupledValue("out_of_plane_strain")
                                                  : adZeroValue()),
    _has_scalar_out_of_plane_strain(isParamValid("scalar_out_of_plane_strain")),
    _nscalar_strains(this->coupledScalarComponents("scalar_out_of_plane_strain"))
{
  if (_has_out_of_plane_strain && _has_scalar_out_of_plane_strain)
    mooseError("Must define only one of out_of_plane_strain or scalar_out_of_plane_strain");

  if (!_has_out_of_plane_strain && !_has_scalar_out_of_plane_strain)
    mooseError("Must define either out_of_plane_strain or scalar_out_of_plane_strain");

  if (_has_scalar_out_of_plane_strain)
  {
    _scalar_out_of_plane_strain.resize(_nscalar_strains);
    for (unsigned int i = 0; i < _nscalar_strains; ++i)
      _scalar_out_of_plane_strain[i] = &this->coupledScalarValue("scalar_out_of_plane_strain", i);
  }
}

template <ComputeStage compute_stage>
void
ADComputeAxisymmetric1DSmallStrain<compute_stage>::initialSetup()
{
  ADComputeStrainBase<compute_stage>::initialSetup();

  if (getBlockCoordSystem() != Moose::COORD_RZ)
    mooseError("The coordinate system must be set to RZ for Axisymmetric geometries.");
}

template <ComputeStage compute_stage>
ADReal
ADComputeAxisymmetric1DSmallStrain<compute_stage>::computeStrainYY()
{
  if (_has_scalar_out_of_plane_strain)
    return (*_scalar_out_of_plane_strain[getCurrentSubblockIndex()])[0];
  else
    return _out_of_plane_strain[_qp];
}

template <ComputeStage compute_stage>
ADReal
ADComputeAxisymmetric1DSmallStrain<compute_stage>::computeStrainZZ()
{
  if (!MooseUtils::absoluteFuzzyEqual(_q_point[_qp](0), 0.0))
    return (*_disp[0])[_qp] / _q_point[_qp](0);
  else
    return 0.0;
}
