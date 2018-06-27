//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputePlaneSmallStrain.h"

registerMooseObject("TensorMechanicsApp", ComputePlaneSmallStrain);

template <>
InputParameters
validParams<ComputePlaneSmallStrain>()
{
  InputParameters params = validParams<Compute2DSmallStrain>();
  params.addClassDescription("Compute a small strain under generalized plane strain assumptions "
                             "where the out of plane strain is generally nonzero.");
  params.addParam<UserObjectName>("subblock_index_provider",
                                  "SubblockIndexProvider user object name");
  params.addCoupledVar("scalar_out_of_plane_strain",
                       "Scalar variable for generalized plane strain");
  params.addCoupledVar("out_of_plane_strain", "Nonlinear variable for plane stress condition");

  return params;
}

ComputePlaneSmallStrain::ComputePlaneSmallStrain(const InputParameters & parameters)
  : Compute2DSmallStrain(parameters),
    _subblock_id_provider(isParamValid("subblock_index_provider")
                              ? &getUserObject<SubblockIndexProvider>("subblock_index_provider")
                              : nullptr),
    _scalar_out_of_plane_strain_coupled(isParamValid("scalar_out_of_plane_strain")),
    _out_of_plane_strain_coupled(isCoupled("out_of_plane_strain")),
    _out_of_plane_strain(_out_of_plane_strain_coupled ? coupledValue("out_of_plane_strain")
                                                      : _zero),
    _nscalar_strains(coupledScalarComponents("scalar_out_of_plane_strain"))
{
  if (_out_of_plane_strain_coupled && _scalar_out_of_plane_strain_coupled)
    mooseError("Must define only one of out_of_plane_strain or scalar_out_of_plane_strain");

  if (_scalar_out_of_plane_strain_coupled)
  {
    _scalar_out_of_plane_strain.resize(_nscalar_strains);
    for (unsigned int i = 0; i < _nscalar_strains; ++i)
      _scalar_out_of_plane_strain[i] = &coupledScalarValue("scalar_out_of_plane_strain", i);
  }
}

Real
ComputePlaneSmallStrain::computeOutOfPlaneStrain()
{
  if (_scalar_out_of_plane_strain_coupled)
    return (*_scalar_out_of_plane_strain[getCurrentSubblockIndex()])[0];
  else
    return _out_of_plane_strain[_qp];
}
