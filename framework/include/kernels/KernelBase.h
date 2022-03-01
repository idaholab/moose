//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ResidualObject.h"
#include "BlockRestrictable.h"
#include "MaterialPropertyInterface.h"
#include "GeometricSearchInterface.h"
#include "ElementIDInterface.h"

/**
 * This is the common base class for the three main
 * kernel types implemented in MOOSE, Kernel, VectorKernel and ArrayKernel.
 */
class KernelBase : public ResidualObject,
                   public BlockRestrictable,
                   public CoupleableMooseVariableDependencyIntermediateInterface,
                   public MaterialPropertyInterface,
                   protected GeometricSearchInterface,
                   public ElementIDInterface
{
public:
  static InputParameters validParams();

  KernelBase(const InputParameters & parameters);

protected:
  /// Current element
  const Elem * const & _current_elem;

  /// Volume of the current element
  const Real & _current_elem_volume;

  /// The current quadrature point index
  unsigned int _qp;

  /// The physical location of the element's quadrature Points, indexed by _qp
  const MooseArray<Point> & _q_point;

  /// active quadrature rule
  const QBase * const & _qrule;

  /// The current quadrature point weight value
  const MooseArray<Real> & _JxW;

  /// The scaling factor to convert from cartesian to another coordinate system (e.g rz, spherical, etc.)
  const MooseArray<Real> & _coord;

  /// current index for the test function
  unsigned int _i;

  /// current index for the shape function
  unsigned int _j;

  /// The aux variables to save the residual contributions to
  bool _has_save_in;
  std::vector<MooseVariableFEBase *> _save_in;
  std::vector<AuxVariableName> _save_in_strings;

  /// The aux variables to save the diagonal Jacobian contributions to
  bool _has_diag_save_in;
  std::vector<MooseVariableFEBase *> _diag_save_in;
  std::vector<AuxVariableName> _diag_save_in_strings;

  std::vector<unsigned int> _displacements;

  /// Whether this object is acting on the displaced mesh
  const bool _use_displaced_mesh;
};
