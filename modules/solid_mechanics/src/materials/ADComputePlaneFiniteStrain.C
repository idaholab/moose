//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputePlaneFiniteStrain.h"
#include "UserObject.h"

registerMooseObject("TensorMechanicsApp", ADComputePlaneFiniteStrain);

InputParameters
ADComputePlaneFiniteStrain::validParams()
{
  InputParameters params = ADCompute2DFiniteStrain::validParams();
  params.addClassDescription("Compute strain increment and rotation increment for finite strain "
                             "under 2D planar assumptions.");
  params.addParam<UserObjectName>("subblock_index_provider",
                                  "SubblockIndexProvider user object name");
  params.addCoupledVar("scalar_out_of_plane_strain",
                       "Scalar variable for generalized plane strain");
  params.addCoupledVar("out_of_plane_strain", "Nonlinear variable for plane stress condition");

  return params;
}

ADComputePlaneFiniteStrain::ADComputePlaneFiniteStrain(const InputParameters & parameters)
  : ADCompute2DFiniteStrain(parameters),
    _subblock_id_provider(isParamValid("subblock_index_provider")
                              ? &getUserObject<SubblockIndexProvider>("subblock_index_provider")
                              : nullptr),
    _scalar_out_of_plane_strain_coupled(isCoupledScalar("scalar_out_of_plane_strain")),
    _out_of_plane_strain_coupled(isCoupled("out_of_plane_strain")),
    _out_of_plane_strain(_out_of_plane_strain_coupled ? adCoupledValue("out_of_plane_strain")
                                                      : _ad_zero),
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
      _scalar_out_of_plane_strain[i] = &adCoupledScalarValue("scalar_out_of_plane_strain", i);
      _scalar_out_of_plane_strain_old[i] = &coupledScalarValueOld("scalar_out_of_plane_strain", i);
    }
  }
}

ADReal
ADComputePlaneFiniteStrain::computeOutOfPlaneGradDisp()
{
  /**
   * This is consistent with the approximation of stretch rate tensor
   * D = log(sqrt(Fhat^T * Fhat)) / dt
   */
  if (_scalar_out_of_plane_strain_coupled)
    return std::exp((*_scalar_out_of_plane_strain[getCurrentSubblockIndex()])[0]) - 1.0;
  else
    return std::exp(_out_of_plane_strain[_qp]) - 1.0;
}

Real
ADComputePlaneFiniteStrain::computeOutOfPlaneGradDispOld()
{
  if (_scalar_out_of_plane_strain_coupled)
    return std::exp((*_scalar_out_of_plane_strain_old[getCurrentSubblockIndex()])[0]) - 1.0;
  else
    return std::exp(_out_of_plane_strain_old[_qp]) - 1.0;
}
