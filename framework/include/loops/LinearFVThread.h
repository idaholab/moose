//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearThread.h"

class LinearFVThread : public LinearThread
{
public:
  LinearFVThread(FEProblemBase & fe_problem);

  // Splitting Constructor
  LinearFVThread(LinearFVThread & x, Threads::split split);

  virtual ~LinearFVThread();

protected:
  ///@{
  /// Base class version just calls compute on each object for the element
  virtual void computeOnElement();
  virtual void computeOnBoundary(BoundaryID bnd_id, const Elem * lower_d_elem);
  virtual void computeOnInterface(BoundaryID bnd_id);
  virtual void computeOnInternalFace(const Elem * neighbor);
  ///@}
};
