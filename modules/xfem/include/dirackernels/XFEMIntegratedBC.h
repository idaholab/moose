//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose Includes
#include "DiracKernel.h"
#include "GeometricCutUserObject.h"

class XFEMIntegratedBC : public DiracKernel
{
public:
  static InputParameters validParams();

  XFEMIntegratedBC(const InputParameters & parameters);

  virtual void addPoints() override;

protected:
  virtual void reinitQp() override;

  /// The kernel is integrated along the interface cut by this geometric cut user object.
  const GeometricCutUserObject & _gcuo;

  /// The interface ID of the cut
  const unsigned int _interface_id;

  /// The map from interface ID to element pair locator
  const std::map<unsigned int, std::shared_ptr<ElementPairLocator>> & _element_pair_locators;

  // The exterior (outward) normal at the quadrature points, this is always pointing from the
  // physical domain to the phantom domain
  std::map<const Elem *, std::map<unsigned int, Point>> _elem_qp_normal;

  /// The quadrature weights at the quadrature points
  std::map<const Elem *, std::map<unsigned int, Real>> _elem_qp_JxW;

  /// The exterior normal at the current quadrature point
  RealVectorValue _interface_normal;
};
