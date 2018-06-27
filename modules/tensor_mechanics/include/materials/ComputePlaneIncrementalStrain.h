//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTEPLANEINCREMENTALSTRAIN_H
#define COMPUTEPLANEINCREMENTALSTRAIN_H

#include "Compute2DIncrementalStrain.h"
#include "SubblockIndexProvider.h"

class ComputePlaneIncrementalStrain;

template <>
InputParameters validParams<ComputePlaneIncrementalStrain>();

/**
 * ComputePlaneIncrementalStrain defines strain increment
 * for small strains in a 2D planar simulation.
 */
class ComputePlaneIncrementalStrain : public Compute2DIncrementalStrain
{
public:
  ComputePlaneIncrementalStrain(const InputParameters & parameters);

protected:
  virtual Real computeOutOfPlaneGradDisp() override;
  virtual Real computeOutOfPlaneGradDispOld() override;

  /// gets its subblock index for current element
  unsigned int getCurrentSubblockIndex() const
  {
    return _subblock_id_provider ? _subblock_id_provider->getSubblockIndex(*_current_elem) : 0;
  };

  const SubblockIndexProvider * _subblock_id_provider;

  const bool _scalar_out_of_plane_strain_coupled;
  unsigned int _nscalar_strains;
  std::vector<const VariableValue *> _scalar_out_of_plane_strain;
  std::vector<const VariableValue *> _scalar_out_of_plane_strain_old;

  const bool _out_of_plane_strain_coupled;
  const VariableValue & _out_of_plane_strain;
  const VariableValue & _out_of_plane_strain_old;
};

#endif // COMPUTEPLANEINCREMENTALSTRAIN_H
