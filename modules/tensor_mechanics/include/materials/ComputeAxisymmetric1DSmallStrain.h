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
#include "SubblockIndexProvider.h"

/**
 * ComputeAxisymmetric1DSmallStrain defines small strains in an Axisymmetric 1D problem.
 * The COORD_TYPE in the Problem block must be set to RZ.
 */
class ComputeAxisymmetric1DSmallStrain : public Compute1DSmallStrain
{
public:
  static InputParameters validParams();

  ComputeAxisymmetric1DSmallStrain(const InputParameters & parameters);

  void initialSetup() override;

protected:
  /// Computes the strain_yy for axisymmetric problems
  Real computeStrainYY() override;

  /// Computes the strain_zz for axisymmetric problems, where
  ///  \f$ \epsilon_{\theta} = \frac{u_r}{r} \f$
  Real computeStrainZZ() override;

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
  const VariableValue & _out_of_plane_strain;

  /// Whether out-of-plane strain scalar variables are coupled
  const bool _has_scalar_out_of_plane_strain;

  /// The out-of-plane strain scalar variables
  std::vector<const VariableValue *> _scalar_out_of_plane_strain;
};
