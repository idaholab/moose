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
#include "libmesh/fe.h"

class SideSetsFromPoints;

template<>
InputParameters validParams<SideSetsFromPoints>();

class SideSetsFromPoints : public AddSideSetsBase
{
public:
  SideSetsFromPoints(const std::string & name, InputParameters parameters);

  virtual ~SideSetsFromPoints();

  virtual void modify();

protected:
  std::vector<Point> _points;

  std::vector<BoundaryID> _boundary_ids;
  std::vector<BoundaryName> _boundary_names;
};

#endif /* SIDESETSFROMPOINTS_H */
