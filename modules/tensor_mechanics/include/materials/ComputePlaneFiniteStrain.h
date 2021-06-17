//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Compute2DFiniteStrain.h"
#include "SubblockIndexProvider.h"

/**
 * ComputePlaneFiniteStrain defines strain increment and rotation
 * increment for finite strain under 2D planar assumptions.
 */
class ComputePlaneFiniteStrain : public Compute2DFiniteStrain
{
public:
  static InputParameters validParams();

  ComputePlaneFiniteStrain(const InputParameters & parameters);

protected:
  virtual Real computeOutOfPlaneGradDisp() override;
  virtual Real computeOutOfPlaneGradDispOld() override;

  /// gets its subblock index for current element
  unsigned int getCurrentSubblockIndex() const
  {
    return _subblock_id_provider ? _subblock_id_provider->getSubblockIndex(*_current_elem) : 0;
  };

  /// A Userobject that carries the subblock ID for all elements
  const SubblockIndexProvider * const _subblock_id_provider;

  /// Whether out-of-plane strain scalar variables are coupled
  const bool _scalar_out_of_plane_strain_coupled;

  ///{@ Current and old values of the out-of-plane strain scalar variable
  std::vector<const VariableValue *> _scalar_out_of_plane_strain;
  std::vector<const VariableValue *> _scalar_out_of_plane_strain_old;
  ///@}

  /// Whether an out-of-plane strain variable is coupled
  const bool _out_of_plane_strain_coupled;

  ///{@ Current and old values of the out-of-plane strain variable
  const VariableValue & _out_of_plane_strain;
  const VariableValue & _out_of_plane_strain_old;
  ///@}
};
