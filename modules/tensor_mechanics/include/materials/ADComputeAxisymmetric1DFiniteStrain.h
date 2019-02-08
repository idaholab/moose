//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADCOMPUTEAXISYMMETRIC1DFINITESTRAIN_H
#define ADCOMPUTEAXISYMMETRIC1DFINITESTRAIN_H

#include "Compute1DFiniteStrain.h"
#include "SubblockIndexProvider.h"

template <ComputeStage>
class ADComputeAxisymmetric1DFiniteStrain;

declareADValidParams(ADComputeAxisymmetric1DFiniteStrain);

/**
 * ADComputeAxisymmetric1DFiniteStrain defines a strain increment for finite strains
 * in an Axisymmetric 1D problem. The COORD_TYPE in the Problem block must be set to RZ.
 */
template <ComputeStage compute_stage>
class ADComputeAxisymmetric1DFiniteStrain : public ADCompute1DFiniteStrain<compute_stage>
{
public:
  ADComputeAxisymmetric1DFiniteStrain(const InputParameters & parameters);

  void initialSetup() override;

protected:
  /// Computes the current dUy/dy for axisymmetric problems
  ADReal computeGradDispYY() override;

  /// Computes the old dUy/dy for axisymmetric problems
  Real computeGradDispYYOld() override;

  /// Computes the current dUz/dz for axisymmetric problems, where
  /// \f$ \epsilon_{\theta} = \frac{u_r}{r} \f$
  ADReal computeGradDispZZ() override;

  /// Computes the old dUz/dz for axisymmetric problems, where
  /// \f$ \epsilon_{\theta-old} = \frac{u_{r-old}}{r_{old}} \f$
  Real computeGradDispZZOld() override;

  /// gets its subblock index for current element
  unsigned int getCurrentSubblockIndex() const
  {
    return _subblock_id_provider ? _subblock_id_provider->getSubblockIndex(*_current_elem) : 0;
  };

  /// the old value of the first component of the displacements vector
  const VariableValue & _disp_old_0;

  const SubblockIndexProvider * _subblock_id_provider;

  bool _has_out_of_plane_strain;
  const ADVariableValue & _out_of_plane_strain;
  const VariableValue & _out_of_plane_strain_old;

  bool _has_scalar_out_of_plane_strain;
  unsigned int _nscalar_strains;
  std::vector<const VariableValue *> _scalar_out_of_plane_strain;
  std::vector<const VariableValue *> _scalar_out_of_plane_strain_old;

  usingCompute1DFiniteStrainMembers;
};

#endif // ADCOMPUTEAXISYMMETRIC1DFINITESTRAIN_H
