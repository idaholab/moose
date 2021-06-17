//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Compute1DIncrementalStrain.h"
#include "SubblockIndexProvider.h"

/**
 * ComputeAxisymmetric1DIncrementalStrain defines a strain increment only
 * for incremental small strains in an Axisymmetric 1D problem.
 * The COORD_TYPE in the Problem block must be set to RZ.
 */
class ComputeAxisymmetric1DIncrementalStrain : public Compute1DIncrementalStrain
{
public:
  static InputParameters validParams();

  ComputeAxisymmetric1DIncrementalStrain(const InputParameters & parameters);

  void initialSetup() override;

protected:
  /// Computes the current dUy/dy for axisymmetric problems
  Real computeGradDispYY() override;

  /// Computes the old dUy/dy for axisymmetric problems
  Real computeGradDispYYOld() override;

  /// Computes the current dUz/dz for axisymmetric problems, where
  ///  \f$ \epsilon_{\theta} = \frac{u_r}{r} \f$
  Real computeGradDispZZ() override;

  /// Computes the old dUz/dz for axisymmetric problems, where
  ///  \f$ \epsilon_{\theta-old} = \frac{u_{r-old}}{r_{old}} \f$
  Real computeGradDispZZOld() override;

  /// gets its subblock index for current element
  unsigned int getCurrentSubblockIndex() const
  {
    return _subblock_id_provider ? _subblock_id_provider->getSubblockIndex(*_current_elem) : 0;
  };

  /// the old value of the first component of the displacements vector
  const VariableValue & _disp_old_0;

  /// A Userobject that carries the subblock ID for all elements
  const SubblockIndexProvider * const _subblock_id_provider;

  /// Whether an out-of-plane strain variable is coupled
  bool _has_out_of_plane_strain;

  ///{@ Current and old values of the out-of-plane strain variable
  const VariableValue & _out_of_plane_strain;
  const VariableValue & _out_of_plane_strain_old;
  ///@}

  /// Whether an out-of-plane strain scalar variable is coupled
  bool _has_scalar_out_of_plane_strain;

  ///{@ Current and old values of the out-of-plane strain scalar variable
  std::vector<const VariableValue *> _scalar_out_of_plane_strain;
  std::vector<const VariableValue *> _scalar_out_of_plane_strain_old;
  ///@}
};
