//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADCompute1DFiniteStrain.h"
#include "SubblockIndexProvider.h"

/**
 * ADComputeAxisymmetric1DFiniteStrain defines a strain increment for finite strains
 * in an Axisymmetric 1D problem. The COORD_TYPE in the Problem block must be set to RZ.
 */
class ADComputeAxisymmetric1DFiniteStrain : public ADCompute1DFiniteStrain
{
public:
  static InputParameters validParams();

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

  /// Gets the subblock index for the current element
  unsigned int getCurrentSubblockIndex() const;

  /// the old value of the first component of the displacements vector
  const VariableValue & _disp_old_0;

  /// A UserObject that carries the subblock ID for all elements
  const SubblockIndexProvider * const _subblock_id_provider;

  /// Whether an out-of-plane strain variable is coupled
  const bool _has_out_of_plane_strain;

  ///@{ Current and old values of the out-of-plane strain variable
  const ADVariableValue & _out_of_plane_strain;
  const VariableValue & _out_of_plane_strain_old;
  ///@}

  /// Whether an out-of-plane strain scalar variable is coupled
  const bool _has_scalar_out_of_plane_strain;

  ///@{ Current and old values of the out-of-plane strain scalar variable
  std::vector<const ADVariableValue *> _scalar_out_of_plane_strain;
  std::vector<const VariableValue *> _scalar_out_of_plane_strain_old;
  ///@}
};
