//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTEAXISYMMETRIC1DSMALLSTRAIN_H
#define COMPUTEAXISYMMETRIC1DSMALLSTRAIN_H

#include "Compute1DSmallStrain.h"
#include "SubblockIndexProvider.h"

class ComputeAxisymmetric1DSmallStrain;

template <>
InputParameters validParams<ComputeAxisymmetric1DSmallStrain>();

/**
 * ComputeAxisymmetric1DSmallStrain defines small strains in an Axisymmetric 1D problem.
 * The COORD_TYPE in the Problem block must be set to RZ.
 */
class ComputeAxisymmetric1DSmallStrain : public Compute1DSmallStrain
{
public:
  ComputeAxisymmetric1DSmallStrain(const InputParameters & parameters);

protected:
  void initialSetup() override;

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

  const SubblockIndexProvider * _subblock_id_provider;

  const bool _has_out_of_plane_strain;
  const VariableValue & _out_of_plane_strain;

  const bool _has_scalar_out_of_plane_strain;
  unsigned int _nscalar_strains;
  std::vector<const VariableValue *> _scalar_out_of_plane_strain;
};

#endif // COMPUTEAXISYMMETRIC1DSMALLSTRAIN_H
