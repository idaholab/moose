//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeAxisymmetric1DIncrementalStrain.h"

registerADMooseObject("TensorMechanicsApp", ADComputeAxisymmetric1DIncrementalStrain);

defineADValidParams(ADComputeAxisymmetric1DIncrementalStrain,
                    ADCompute1DIncrementalStrain,
                    params.addClassDescription(
                        "Compute strain increment for small strains in an axisymmetric 1D problem");
                    params.addParam<UserObjectName>("subblock_index_provider",
                                                    "SubblockIndexProvider user object name");
                    params.addCoupledVar("scalar_out_of_plane_strain",
                                         "Scalar variable for axisymmetric 1D problem");
                    params.addCoupledVar("out_of_plane_strain",
                                         "Nonlinear variable for axisymmetric 1D problem"););

template <ComputeStage compute_stage>
ADComputeAxisymmetric1DIncrementalStrain<compute_stage>::ADComputeAxisymmetric1DIncrementalStrain(
    const InputParameters & parameters)
  : ADCompute1DIncrementalStrain<compute_stage>(parameters),
    _disp_old_0(coupledValueOld("displacements", 0)),
    _subblock_id_provider(
        isParamValid("subblock_index_provider")
            ? &this->template getUserObject<SubblockIndexProvider>("subblock_index_provider")
            : nullptr),
    _has_out_of_plane_strain(isParamValid("out_of_plane_strain")),
    _out_of_plane_strain(_has_out_of_plane_strain ? adCoupledValue("out_of_plane_strain")
                                                  : adZeroValue()),
    _out_of_plane_strain_old(_has_out_of_plane_strain ? coupledValueOld("out_of_plane_strain")
                                                      : _zero),
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
    _scalar_out_of_plane_strain_old.resize(_nscalar_strains);
    for (unsigned int i = 0; i < _nscalar_strains; ++i)
    {
      _scalar_out_of_plane_strain[i] = &this->coupledScalarValue("scalar_out_of_plane_strain", i);
      _scalar_out_of_plane_strain_old[i] =
          &this->coupledScalarValueOld("scalar_out_of_plane_strain", i);
    }
  }
}

template <ComputeStage compute_stage>
void
ADComputeAxisymmetric1DIncrementalStrain<compute_stage>::initialSetup()
{
  ADComputeIncrementalStrainBase<compute_stage>::initialSetup();

  if (getBlockCoordSystem() != Moose::COORD_RZ)
    mooseError("The coordinate system must be set to RZ for Axisymmetric 1D simulations");
}

template <ComputeStage compute_stage>
ADReal
ADComputeAxisymmetric1DIncrementalStrain<compute_stage>::computeGradDispYY()
{
  if (_has_scalar_out_of_plane_strain)
    return (*_scalar_out_of_plane_strain[getCurrentSubblockIndex()])[0];
  else
    return _out_of_plane_strain[_qp];
}

template <ComputeStage compute_stage>
Real
ADComputeAxisymmetric1DIncrementalStrain<compute_stage>::computeGradDispYYOld()
{
  if (_has_scalar_out_of_plane_strain)
    return (*_scalar_out_of_plane_strain_old[getCurrentSubblockIndex()])[0];
  else
    return _out_of_plane_strain_old[_qp];
}

template <ComputeStage compute_stage>
ADReal
ADComputeAxisymmetric1DIncrementalStrain<compute_stage>::computeGradDispZZ()
{
  if (!MooseUtils::absoluteFuzzyEqual(_q_point[_qp](0), 0.0))
    return (*_disp[0])[_qp] / _q_point[_qp](0);
  else
    return 0.0;
}

template <ComputeStage compute_stage>
Real
ADComputeAxisymmetric1DIncrementalStrain<compute_stage>::computeGradDispZZOld()
{
  if (!MooseUtils::absoluteFuzzyEqual(_q_point[_qp](0), 0.0))
    return _disp_old_0[_qp] / _q_point[_qp](0);
  else
    return 0.0;
}
