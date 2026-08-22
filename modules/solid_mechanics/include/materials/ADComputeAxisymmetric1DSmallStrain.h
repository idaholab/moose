//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADCompute1DSmallStrain.h"
#include "SubblockIndexProvider.h"

/**
 * ADComputeAxisymmetric1DSmallStrain defines small strains in an Axisymmetric 1D problem.
 * The COORD_TYPE in the Problem block must be set to RZ.
 */
class ADComputeAxisymmetric1DSmallStrain : public ADCompute1DSmallStrain
{
public:
  static InputParameters validParams();

  ADComputeAxisymmetric1DSmallStrain(const InputParameters & parameters);

  void initialSetup() override;

protected:
  /// Computes the strain_yy for axisymmetric problems
  ADReal computeStrainYY() override;

  /// Computes the strain_zz for axisymmetric problems, where
  ///  \f$ \epsilon_{\theta} = \frac{u_r}{r} \f$
  ADReal computeStrainZZ() override;

  /// Gets the subblock index for the current element
  unsigned int getCurrentSubblockIndex() const;

  /// A UserObject that carries the subblock ID for all elements
  const SubblockIndexProvider * const _subblock_id_provider;

  /// Whether an out-of-plane strain variable is coupled
  const bool _has_out_of_plane_strain;

  /// The out-of-plane strain variable
  const ADVariableValue & _out_of_plane_strain;

  /// Whether out-of-plane strain scalar variables are coupled
  const bool _has_scalar_out_of_plane_strain;

  /// The out-of-plane strain scalar variables
  std::vector<const ADVariableValue *> _scalar_out_of_plane_strain;
};
