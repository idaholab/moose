//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputePlaneIncrementalStrain.h"

registerADMooseObject("TensorMechanicsApp", ADComputePlaneIncrementalStrain);

defineADLegacyParams(ADComputePlaneIncrementalStrain);

template <ComputeStage compute_stage>
InputParameters
ADComputePlaneIncrementalStrain<compute_stage>::validParams()
{
  InputParameters params = ADCompute2DIncrementalStrain<compute_stage>::validParams();
  params.addClassDescription(
      "Compute strain increment for small strain under 2D planar assumptions.");
  params.addParam<UserObjectName>("subblock_index_provider",
                                  "SubblockIndexProvider user object name");
  params.addCoupledVar("scalar_out_of_plane_strain",
                       "Scalar variable for generalized plane strain");
  params.addCoupledVar("out_of_plane_strain", "Nonlinear variable for plane stress condition");

  return params;
}

template <ComputeStage compute_stage>
ADComputePlaneIncrementalStrain<compute_stage>::ADComputePlaneIncrementalStrain(
    const InputParameters & parameters)
  : ADCompute2DIncrementalStrain<compute_stage>(parameters),
    _subblock_id_provider(isParamValid("subblock_index_provider")
                              ? &getUserObject<SubblockIndexProvider>("subblock_index_provider")
                              : nullptr),
    _scalar_out_of_plane_strain_coupled(isParamValid("scalar_out_of_plane_strain")),
    _nscalar_strains(coupledScalarComponents("scalar_out_of_plane_strain")),
    _out_of_plane_strain_coupled(isCoupled("out_of_plane_strain")),
    _out_of_plane_strain(_out_of_plane_strain_coupled ? adCoupledValue("out_of_plane_strain")
                                                      : adZeroValue()),
    _out_of_plane_strain_old(_out_of_plane_strain_coupled ? coupledValueOld("out_of_plane_strain")
                                                          : _zero)
{
  if (_out_of_plane_strain_coupled && _scalar_out_of_plane_strain_coupled)
    mooseError("Must define only one of out_of_plane_strain or scalar_out_of_plane_strain");

  if (_scalar_out_of_plane_strain_coupled)
  {
    _scalar_out_of_plane_strain.resize(_nscalar_strains);
    _scalar_out_of_plane_strain_old.resize(_nscalar_strains);
    for (unsigned int i = 0; i < _nscalar_strains; ++i)
    {
      _scalar_out_of_plane_strain[i] = &adCoupledScalarValue("scalar_out_of_plane_strain", i);
      _scalar_out_of_plane_strain_old[i] = &coupledScalarValueOld("scalar_out_of_plane_strain", i);
    }
  }
}

template <ComputeStage compute_stage>
ADReal
ADComputePlaneIncrementalStrain<compute_stage>::computeOutOfPlaneGradDisp()
{
  if (_scalar_out_of_plane_strain_coupled)
    return (*_scalar_out_of_plane_strain[getCurrentSubblockIndex()])[0];
  else
    return _out_of_plane_strain[_qp];
}

template <ComputeStage compute_stage>
Real
ADComputePlaneIncrementalStrain<compute_stage>::computeOutOfPlaneGradDispOld()
{
  if (_scalar_out_of_plane_strain_coupled)
    return (*_scalar_out_of_plane_strain_old[getCurrentSubblockIndex()])[0];
  else
    return _out_of_plane_strain_old[_qp];
}
