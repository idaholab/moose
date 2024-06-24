//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PeridynamicsKernelBase.h"
#include "DerivativeMaterialInterface.h"

/**
 * Base kernel class for peridynamic solid mechanics models
 */
class MechanicsBasePD : public DerivativeMaterialInterface<PeridynamicsKernelBase>
{
public:
  static InputParameters validParams();

  MechanicsBasePD(const InputParameters & parameters);

  virtual void computeOffDiagJacobian(unsigned int jvar) override;

  /**
   * Function to compute local contribution to the off-diagonal Jacobian at the current nodes
   * @param coupled_component   The coupled variable number
   */
  virtual void computeLocalOffDiagJacobian(unsigned int /* jvar_num */,
                                           unsigned int /* coupled_component */){};

  /**
   * Function to compute nonlocal contribution to the off-diagonal Jacobian at the current nodes
   * @param jvar_num   The number of the first coupled variable
   * @param coupled_component   The component number of the second coupled variable
   */
  virtual void computePDNonlocalOffDiagJacobian(unsigned int /* jvar_num */,
                                                unsigned int /* coupled_component */){};

  virtual void initialSetup() override;
  virtual void prepare() override;

protected:
  /// displacement variables
  std::vector<MooseVariable *> _disp_var;

  ///@{ Temperature variable
  const bool _temp_coupled;
  MooseVariable * _temp_var;
  ///@}

  /// number of displacement components
  unsigned int _ndisp;

  ///@{ Parameters for out-of-plane strain in weak plane stress formulation
  const bool _out_of_plane_strain_coupled;
  MooseVariable * _out_of_plane_strain_var;
  ///@}

  /// Vector of bond in current configuration
  const std::vector<RealGradient> * _orientation;

  /// Current variable dof numbers for nodes i and j
  std::vector<dof_id_type> _ivardofs;

  /// weights used for the current element to obtain the nodal stress
  std::vector<Real> _weights;

  /// Vector of bond in current configuration
  RealGradient _current_vec;

  /// Unit vector of bond in current configuration
  RealGradient _current_unit_vec;
};
