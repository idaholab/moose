//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BoundaryCondition.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "MaterialPropertyInterface.h"

/**
 * Base class for deriving any boundary condition of a integrated type
 */
class IntegratedBCBase : public BoundaryCondition,
                         public CoupleableMooseVariableDependencyIntermediateInterface,
                         public MaterialPropertyInterface
{
public:
  static InputParameters validParams();

  IntegratedBCBase(const InputParameters & parameters);

  void prepareShapes(unsigned int var_num) override final;

protected:
  /// current element
  const Elem * const & _current_elem;
  /// Volume of the current element
  const Real & _current_elem_volume;
  /// current side of the current element
  const unsigned int & _current_side;
  /// current side element
  const Elem * const & _current_side_elem;
  /// Volume of the current side
  const Real & _current_side_volume;
  /// The currenty boundary id
  const BoundaryID & _current_boundary_id;

  /// quadrature point index
  unsigned int _qp;
  /// active quadrature rule
  const QBase * const & _qrule;
  /// active quadrature points
  const MooseArray<Point> & _q_point;
  /// transformed Jacobian weights
  const MooseArray<Real> & _JxW;
  /// coordinate transformation
  const MooseArray<Real> & _coord;
  /// i-th, j-th index for enumerating test and shape functions
  unsigned int _i, _j;

  /// The aux variables to save the residual contributions to
  bool _has_save_in;
  std::vector<MooseVariableFEBase *> _save_in;
  std::vector<AuxVariableName> _save_in_strings;

  /// The aux variables to save the diagonal Jacobian contributions to
  bool _has_diag_save_in;
  std::vector<MooseVariableFEBase *> _diag_save_in;
  std::vector<AuxVariableName> _diag_save_in_strings;
};
