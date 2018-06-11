//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTEPLANESMALLSTRAIN_H
#define COMPUTEPLANESMALLSTRAIN_H

#include "Compute2DSmallStrain.h"
#include "SubblockIndexProvider.h"

class ComputePlaneSmallStrain;

template <>
InputParameters validParams<ComputePlaneSmallStrain>();

/**
 * ComputePlaneSmallStrain defines small strains under generalized
 * plane strain and plane stress assumptions, where the out of plane strain
 * can be uniformly or non-uniformly zero or nonzero.
 */
class ComputePlaneSmallStrain : public Compute2DSmallStrain
{
public:
  ComputePlaneSmallStrain(const InputParameters & parameters);

protected:
  virtual Real computeOutOfPlaneStrain();

  /// gets its subblock index for current element
  unsigned int getCurrentSubblockIndex() const
  {
    return _subblock_id_provider ? _subblock_id_provider->getSubblockIndex(*_current_elem) : 0;
  };

  const SubblockIndexProvider * _subblock_id_provider;

private:
  const bool _scalar_out_of_plane_strain_coupled;

  const bool _out_of_plane_strain_coupled;
  const VariableValue & _out_of_plane_strain;
  unsigned int _nscalar_strains;
  std::vector<const VariableValue *> _scalar_out_of_plane_strain;
};

#endif // COMPUTEPLANESMALLSTRAIN_H
