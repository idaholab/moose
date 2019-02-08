//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADCOMPUTEAXISYMMETRIC1DSMALLSTRAIN_H
#define ADCOMPUTEAXISYMMETRIC1DSMALLSTRAIN_H

#include "ADCompute1DSmallStrain.h"
#include "SubblockIndexProvider.h"

template <ComputeStage>
class ADComputeAxisymmetric1DSmallStrain;

declareADValidParams(ADComputeAxisymmetric1DSmallStrain);

/**
 * ADComputeAxisymmetric1DSmallStrain defines small strains in an Axisymmetric 1D problem.
 * The COORD_TYPE in the Problem block must be set to RZ.
 */
template <ComputeStage compute_stage>
class ADComputeAxisymmetric1DSmallStrain : public ADCompute1DSmallStrain<compute_stage>
{
public:
  ADComputeAxisymmetric1DSmallStrain(const InputParameters & parameters);

  void initialSetup() override;

protected:
  /// Computes the strain_yy for axisymmetric problems
  ADReal computeStrainYY() override;

  /// Computes the strain_zz for axisymmetric problems, where
  ///  \f$ \epsilon_{\theta} = \frac{u_r}{r} \f$
  ADReal computeStrainZZ() override;

  /// gets its subblock index for current element
  unsigned int getCurrentSubblockIndex() const
  {
    return _subblock_id_provider ? _subblock_id_provider->getSubblockIndex(*_current_elem) : 0;
  };

  const SubblockIndexProvider * _subblock_id_provider;

  const bool _has_out_of_plane_strain;
  const ADVariableValue & _out_of_plane_strain;

  const bool _has_scalar_out_of_plane_strain;
  unsigned int _nscalar_strains;
  std::vector<const VariableValue *> _scalar_out_of_plane_strain;

  usingCompute1DSmallStrainMembers;
};

#endif // ADCOMPUTEAXISYMMETRIC1DSMALLSTRAIN_H
