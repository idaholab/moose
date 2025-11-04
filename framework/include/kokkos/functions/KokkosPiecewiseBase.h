//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosFunction.h"

/**
 * Function base which provides a piecewise approximation to a specified (x,y) point data set.
 * Derived classes can either directly implement the x/y data, or provide input parameter mechanisms
 * for such data formulation.
 */
class KokkosPiecewiseBase : public Moose::Kokkos::FunctionBase
{
public:
  static InputParameters validParams();

  KokkosPiecewiseBase(const InputParameters & parameters);

  KOKKOS_FUNCTION dof_id_type functionSize() const { return _raw_x.size(); }
  KOKKOS_FUNCTION Real domain(const unsigned int i) const { return _raw_x[i]; }
  KOKKOS_FUNCTION Real range(const unsigned int i) const { return _raw_y[i]; }

  /**
   * Provides a means for explicitly setting the x and y data. This must
   * be called in the constructor of inherited classes.
   */
  virtual void setData(const std::vector<Real> & x, const std::vector<Real> & y);

protected:
  ///@{ raw function data as read
  Moose::Kokkos::Array<Real> _raw_x;
  Moose::Kokkos::Array<Real> _raw_y;
  ///@}
};
