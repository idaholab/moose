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

#ifndef SIDESETSFROMPOINTS_H
#define SIDESETSFROMPOINTS_H

#include "AddSideSetsBase.h"

class SideSetsFromPoints;

template <>
InputParameters validParams<SideSetsFromPoints>();

class SideSetsFromPoints : public AddSideSetsBase
{
public:
  SideSetsFromPoints(const InputParameters & parameters);

protected:
  virtual void modify() override;

  std::vector<BoundaryName> _boundary_names;

  std::vector<Point> _points;
};

#endif /* SIDESETSFROMPOINTS_H */
