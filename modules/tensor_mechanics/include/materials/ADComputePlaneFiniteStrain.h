//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADCompute2DFiniteStrain.h"
#include "SubblockIndexProvider.h"

/**
 * ADComputePlaneFiniteStrain defines strain increment and rotation
 * increment for finite strain under 2D planar assumptions.
 */
class ADComputePlaneFiniteStrain : public ADCompute2DFiniteStrain
{
public:
  static InputParameters validParams();

  ADComputePlaneFiniteStrain(const InputParameters & parameters);

protected:
  virtual ADReal computeOutOfPlaneGradDisp() override;
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
  std::vector<const ADVariableValue *> _scalar_out_of_plane_strain;
  std::vector<const VariableValue *> _scalar_out_of_plane_strain_old;
  ///@}

  /// Whether out-of-plane strain variables are coupled
  const bool _out_of_plane_strain_coupled;

  ///{@ Current and old values of the out-of-plane strain variable
  const ADVariableValue & _out_of_plane_strain;
  const VariableValue & _out_of_plane_strain_old;
  ///@}
};
