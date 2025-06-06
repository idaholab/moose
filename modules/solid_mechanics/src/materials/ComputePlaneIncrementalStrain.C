//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputePlaneIncrementalStrain.h"
#include "UserObject.h"

registerMooseObject("SolidMechanicsApp", ComputePlaneIncrementalStrain);

InputParameters
ComputePlaneIncrementalStrain::validParams()
{
  InputParameters params = Compute2DIncrementalStrain::validParams();
  params.addClassDescription(
      "Compute strain increment for small strain under 2D planar assumptions.");
  params.addParam<UserObjectName>("subblock_index_provider",
                                  "SubblockIndexProvider user object name");
  params.addCoupledVar("scalar_out_of_plane_strain",
                       "Scalar variable for generalized plane strain");
  params.addCoupledVar("out_of_plane_strain", "Nonlinear variable for plane stress condition");

  return params;
}

ComputePlaneIncrementalStrain::ComputePlaneIncrementalStrain(const InputParameters & parameters)
  : Compute2DIncrementalStrain(parameters),
    _subblock_id_provider(isParamValid("subblock_index_provider")
                              ? &getUserObject<SubblockIndexProvider>("subblock_index_provider")
                              : nullptr),
    _scalar_out_of_plane_strain_coupled(isCoupledScalar("scalar_out_of_plane_strain")),
    _out_of_plane_strain_coupled(isCoupled("out_of_plane_strain")),
    _out_of_plane_strain(_out_of_plane_strain_coupled ? coupledValue("out_of_plane_strain")
                                                      : _zero),
    _out_of_plane_strain_old(_out_of_plane_strain_coupled ? coupledValueOld("out_of_plane_strain")
                                                          : _zero)
{
  if (_out_of_plane_strain_coupled && _scalar_out_of_plane_strain_coupled)
    mooseError("Must define only one of out_of_plane_strain or scalar_out_of_plane_strain");

  if (_scalar_out_of_plane_strain_coupled)
  {
    const auto nscalar_strains = coupledScalarComponents("scalar_out_of_plane_strain");
    _scalar_out_of_plane_strain.resize(nscalar_strains);
    _scalar_out_of_plane_strain_old.resize(nscalar_strains);
    for (unsigned int i = 0; i < nscalar_strains; ++i)
    {
      _scalar_out_of_plane_strain[i] = &coupledScalarValue("scalar_out_of_plane_strain", i);
      _scalar_out_of_plane_strain_old[i] = &coupledScalarValueOld("scalar_out_of_plane_strain", i);
    }
  }
}

Real
ComputePlaneIncrementalStrain::computeOutOfPlaneGradDisp()
{
  if (_scalar_out_of_plane_strain_coupled)
    return (*_scalar_out_of_plane_strain[getCurrentSubblockIndex()])[0];
  else
    return _out_of_plane_strain[_qp];
}

Real
ComputePlaneIncrementalStrain::computeOutOfPlaneGradDispOld()
{
  if (_scalar_out_of_plane_strain_coupled)
    return (*_scalar_out_of_plane_strain_old[getCurrentSubblockIndex()])[0];
  else
    return _out_of_plane_strain_old[_qp];
}
