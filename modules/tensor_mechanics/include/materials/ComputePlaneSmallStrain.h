//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Compute2DSmallStrain.h"
#include "SubblockIndexProvider.h"

/**
 * ComputePlaneSmallStrain defines small strains under generalized
 * plane strain and plane stress assumptions, where the out of plane strain
 * can be uniformly or non-uniformly zero or nonzero.
 */
class ComputePlaneSmallStrain : public Compute2DSmallStrain
{
public:
  static InputParameters validParams();

  ComputePlaneSmallStrain(const InputParameters & parameters);

protected:
  virtual Real computeOutOfPlaneStrain();

  /// gets its subblock index for current element
  unsigned int getCurrentSubblockIndex() const
  {
    return _subblock_id_provider ? _subblock_id_provider->getSubblockIndex(*_current_elem) : 0;
  };

  /// A Userobject that carries the subblock ID for all elements
  const SubblockIndexProvider * const _subblock_id_provider;

private:
  /// Whether out-of-plane strain scalar variables are coupled
  const bool _scalar_out_of_plane_strain_coupled;

  /// Whether an out-of-plane strain variable is coupled
  const bool _out_of_plane_strain_coupled;

  /// The out-of-plane strain variable
  const VariableValue & _out_of_plane_strain;

  /// The out-of-plane strain scalar variables
  std::vector<const VariableValue *> _scalar_out_of_plane_strain;
};
