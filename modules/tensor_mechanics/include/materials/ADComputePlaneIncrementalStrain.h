//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADCompute2DIncrementalStrain.h"
#include "SubblockIndexProvider.h"

/**
 * ADComputePlaneIncrementalStrain defines strain increment
 * for small strains in a 2D planar simulation.
 */
class ADComputePlaneIncrementalStrain : public ADCompute2DIncrementalStrain
{
public:
  static InputParameters validParams();

  ADComputePlaneIncrementalStrain(const InputParameters & parameters);

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

  /// Whether an out-of-plane strain variable is coupled
  const bool _out_of_plane_strain_coupled;

  ///{@ Current and old values of the out-of-plane strain variable
  const ADVariableValue & _out_of_plane_strain;
  const VariableValue & _out_of_plane_strain_old;
  ///@}
};
