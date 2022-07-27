//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Compute1DFiniteStrain.h"
#include "ADCompute1DFiniteStrain.h"
#include "SubblockIndexProvider.h"

// select the appropriate class based on the is_ad boolean parameter
template <bool is_ad>
using Generic1DFiniteStrain =
    typename std::conditional<is_ad, ADCompute1DFiniteStrain, Compute1DFiniteStrain>::type;

template <bool is_ad>
using GenericComputeIncrementalStrainBase = typename std::
    conditional<is_ad, ADComputeIncrementalStrainBase, ComputeIncrementalStrainBase>::type;

/**
 * ComputeAxisymmetric1DFiniteStrain defines a strain increment for finite strains
 * in an Axisymmetric 1D problem. The COORD_TYPE in the Mesh block must be set to RZ.
 */
template <bool is_ad>
class ComputeAxisymmetric1DFiniteStrainTempl : public Generic1DFiniteStrain<is_ad>
{
public:
  static InputParameters validParams();

  ComputeAxisymmetric1DFiniteStrainTempl(const InputParameters & parameters);

  void initialSetup() override;

protected:
  /// Computes the current dUy/dy for axisymmetric problems
  GenericReal<is_ad> computeGradDispYY() override;

  /// Computes the old dUy/dy for axisymmetric problems
  Real computeGradDispYYOld() override;

  /// Computes the current dUz/dz for axisymmetric problems, where
  /// \f$ \epsilon_{\theta} = \frac{u_r}{r} \f$
  GenericReal<is_ad> computeGradDispZZ() override;

  /// Computes the old dUz/dz for axisymmetric problems, where
  /// \f$ \epsilon_{\theta-old} = \frac{u_{r-old}}{r_{old}} \f$
  Real computeGradDispZZOld() override;

  /// gets its subblock index for current element
  unsigned int getCurrentSubblockIndex() const
  {
    return _subblock_id_provider ? _subblock_id_provider->getSubblockIndex(*_current_elem) : 0;
  };

  /// the old value of the first component of the displacements vector
  const VariableValue & _disp_old_0;

  /// A Userobject that carries the subblock ID for all elements
  const SubblockIndexProvider * const _subblock_id_provider;

  /// Whether an out-of-plane strain variable is coupled
  bool _has_out_of_plane_strain;

  ///{@ Current and old values of the out-of-plane strain variable
  const GenericVariableValue<is_ad> & _out_of_plane_strain;
  const VariableValue & _out_of_plane_strain_old;
  ///@}

  /// Whether an out-of-plane strain scalar variable is coupled
  bool _has_scalar_out_of_plane_strain;

  /// Number of out-of-plane strain scalar variables
  unsigned int _nscalar_strains;

  ///{@ Current and old values of the out-of-plane strain scalar variable
  std::vector<const GenericVariableValue<is_ad> *> _scalar_out_of_plane_strain;
  std::vector<const VariableValue *> _scalar_out_of_plane_strain_old;
  ///@}

  using Material::_current_elem;
  using Material::_q_point;
  using Material::_qp;
  using Material::coupledScalarComponents;
  using Material::coupledScalarValueOld;
  using Material::coupledValueOld;
  using Material::getBlockCoordSystem;
  using Material::isCoupled;
  using Material::isCoupledScalar;
  using Material::isParamValid;
  using GenericComputeIncrementalStrainBase<is_ad>::_disp;
  using GenericComputeIncrementalStrainBase<is_ad>::coupledScalarValue;
  using GenericComputeIncrementalStrainBase<is_ad>::adCoupledScalarValue;
};

typedef ComputeAxisymmetric1DFiniteStrainTempl<false> ComputeAxisymmetric1DFiniteStrain;
typedef ComputeAxisymmetric1DFiniteStrainTempl<true> ADComputeAxisymmetric1DFiniteStrain;
