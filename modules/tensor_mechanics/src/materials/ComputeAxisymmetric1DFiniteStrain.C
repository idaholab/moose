//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeAxisymmetric1DFiniteStrain.h"
#include "UserObject.h"

registerMooseObject("TensorMechanicsApp", ComputeAxisymmetric1DFiniteStrain);
registerMooseObject("TensorMechanicsApp", ADComputeAxisymmetric1DFiniteStrain);

template <bool is_ad>
InputParameters
ComputeAxisymmetric1DFiniteStrainTempl<is_ad>::validParams()
{
  InputParameters params = Generic1DFiniteStrain<is_ad>::validParams();
  params.addClassDescription("Compute a strain increment and rotation increment for finite strains "
                             "in an axisymmetric 1D problem");
  params.addParam<UserObjectName>("subblock_index_provider",
                                  "SubblockIndexProvider user object name");
  params.addCoupledVar("scalar_out_of_plane_strain", "Scalar variable for axisymmetric 1D problem");
  params.addCoupledVar("out_of_plane_strain", "Nonlinear variable for axisymmetric 1D problem");

  return params;
}

template <bool is_ad>
ComputeAxisymmetric1DFiniteStrainTempl<is_ad>::ComputeAxisymmetric1DFiniteStrainTempl(
    const InputParameters & parameters)
  : Generic1DFiniteStrain<is_ad>(parameters),
    _disp_old_0(coupledValueOld("displacements", 0)),
    _subblock_id_provider(
        isParamValid("subblock_index_provider")
            ? &this->template getUserObject<SubblockIndexProvider>("subblock_index_provider")
            : nullptr),
    _has_out_of_plane_strain(isCoupled("out_of_plane_strain")),
    _out_of_plane_strain(_has_out_of_plane_strain
                             ? this->template coupledGenericValue<is_ad>("out_of_plane_strain")
                             : this->template genericZeroValue<is_ad>()),
    _out_of_plane_strain_old(_has_out_of_plane_strain ? coupledValueOld("out_of_plane_strain")
                                                      : this->template genericZeroValue<false>()),
    _has_scalar_out_of_plane_strain(isCoupledScalar("scalar_out_of_plane_strain"))
{
  if (_has_out_of_plane_strain && _has_scalar_out_of_plane_strain)
    mooseError("Must define only one of out_of_plane_strain or scalar_out_of_plane_strain");

  if (_has_scalar_out_of_plane_strain)
  {
    const auto nscalar_strains = coupledScalarComponents("scalar_out_of_plane_strain");
    _scalar_out_of_plane_strain.resize(nscalar_strains);
    _scalar_out_of_plane_strain_old.resize(nscalar_strains);

    for (unsigned int i = 0; i < nscalar_strains; ++i)
    {
      if constexpr (is_ad)
        _scalar_out_of_plane_strain[i] = &adCoupledScalarValue("scalar_out_of_plane_strain", i);
      else
        _scalar_out_of_plane_strain[i] = &coupledScalarValue("scalar_out_of_plane_strain", i);

      _scalar_out_of_plane_strain_old[i] = &coupledScalarValueOld("scalar_out_of_plane_strain", i);
    }
  }
}

template <bool is_ad>
void
ComputeAxisymmetric1DFiniteStrainTempl<is_ad>::initialSetup()
{
  GenericComputeIncrementalStrainBase<is_ad>::initialSetup();

  if (getBlockCoordSystem() != Moose::COORD_RZ)
    mooseError("The coordinate system must be set to RZ for Axisymmetric geometries.");
}

template <bool is_ad>
GenericReal<is_ad>
ComputeAxisymmetric1DFiniteStrainTempl<is_ad>::computeGradDispYY()
{
  if (_has_scalar_out_of_plane_strain)
    return std::exp((*_scalar_out_of_plane_strain[getCurrentSubblockIndex()])[0]) - 1.0;
  else
    return std::exp(_out_of_plane_strain[_qp]) - 1.0;
}

template <bool is_ad>
Real
ComputeAxisymmetric1DFiniteStrainTempl<is_ad>::computeGradDispYYOld()
{
  if (_has_scalar_out_of_plane_strain)
    return std::exp((*_scalar_out_of_plane_strain_old[getCurrentSubblockIndex()])[0]) - 1.0;
  else
    return std::exp(_out_of_plane_strain_old[_qp]) - 1.0;
}

template <bool is_ad>
GenericReal<is_ad>
ComputeAxisymmetric1DFiniteStrainTempl<is_ad>::computeGradDispZZ()
{
  if (!MooseUtils::absoluteFuzzyEqual(_q_point[_qp](0), 0.0))
    return (*_disp[0])[_qp] / _q_point[_qp](0);
  else
    return 0.0;
}

template <bool is_ad>
Real
ComputeAxisymmetric1DFiniteStrainTempl<is_ad>::computeGradDispZZOld()
{
  if (!MooseUtils::absoluteFuzzyEqual(_q_point[_qp](0), 0.0))
    return _disp_old_0[_qp] / _q_point[_qp](0);
  else
    return 0.0;
}
