/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef DIRACKERNELINFO_H
#define DIRACKERNELINFO_H

#include "Moose.h"
#include "MooseArray.h"

//Forward Declarations
class MooseSystem;

namespace libMesh
{
  template <class T> class NumericVector;
}

class DiracKernelInfo
{
public:
  DiracKernelInfo(MooseSystem & moose_system);
  virtual ~DiracKernelInfo();

public:
  void addPoint(const Elem * elem, Point p);

  /**
   * The MooseSystem this DiracKernel is associated with.
   */
  MooseSystem & _moose_system;

  /**
   * The list of elements that need distributions.
   */
  std::set<const Elem *> _elements;

  /**
   * The list of physical xyz Points that need to be evaluated in each element.
   */
  std::map<const Elem *, std::set<Point> > _points;
};


#endif //DIRACKERNELINFO_H
