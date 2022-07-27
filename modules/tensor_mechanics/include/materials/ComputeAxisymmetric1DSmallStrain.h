//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Compute1DSmallStrain.h"
#include "ADCompute1DSmallStrain.h"
#include "SubblockIndexProvider.h"

// select the appropriate class based on the is_ad boolean parameter
template <bool is_ad>
using Generic1DSmallStrain =
    typename std::conditional<is_ad, ADCompute1DSmallStrain, Compute1DSmallStrain>::type;

template <bool is_ad>
using GenericComputeStrainBase =
    typename std::conditional<is_ad, ADComputeStrainBase, ComputeStrainBase>::type;

/**
 * ComputeAxisymmetric1DSmallStrain defines small strains in an Axisymmetric 1D problem.
 * The COORD_TYPE in the Mesh block must be set to RZ.
 */
template <bool is_ad>
class ComputeAxisymmetric1DSmallStrainTempl : public Generic1DSmallStrain<is_ad>
{
public:
  static InputParameters validParams();

  ComputeAxisymmetric1DSmallStrainTempl(const InputParameters & parameters);

  void initialSetup() override;

protected:
  /// Computes the strain_yy for axisymmetric problems
  GenericReal<is_ad> computeStrainYY() override;

  /// Computes the strain_zz for axisymmetric problems, where
  ///  \f$ \epsilon_{\theta} = \frac{u_r}{r} \f$
  GenericReal<is_ad> computeStrainZZ() override;

  /// gets its subblock index for current element
  unsigned int getCurrentSubblockIndex() const
  {
    return _subblock_id_provider ? _subblock_id_provider->getSubblockIndex(*_current_elem) : 0;
  };

  /// A Userobject that carries the subblock ID for all elements
  const SubblockIndexProvider * const _subblock_id_provider;

  /// Whether an out-of-plane strain variable is coupled
  const bool _has_out_of_plane_strain;

  /// The out-of-plane strain variable
  const GenericVariableValue<is_ad> & _out_of_plane_strain;

  /// Whether out-of-plane strain scalar variables are coupled
  const bool _has_scalar_out_of_plane_strain;

  /// The out-of-plane strain scalar variables
  std::vector<const GenericVariableValue<is_ad> *> _scalar_out_of_plane_strain;

  using Material::_current_elem;
  using Material::_q_point;
  using Material::_qp;
  using Material::coupledScalarComponents;
  using Material::getBlockCoordSystem;
  using Material::isCoupled;
  using Material::isCoupledScalar;
  using Material::isParamValid;
  using GenericComputeStrainBase<is_ad>::_disp;
  using GenericComputeStrainBase<is_ad>::coupledScalarValue;
  using GenericComputeStrainBase<is_ad>::adCoupledScalarValue;
};

typedef ComputeAxisymmetric1DSmallStrainTempl<false> ComputeAxisymmetric1DSmallStrain;
typedef ComputeAxisymmetric1DSmallStrainTempl<true> ADComputeAxisymmetric1DSmallStrain;
