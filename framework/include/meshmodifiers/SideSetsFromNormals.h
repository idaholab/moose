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

#ifndef SIDESETSFROMNORMALS_H
#define SIDESETSFROMNORMALS_H

#include "AddSideSetsBase.h"

class SideSetsFromNormals;

template <>
InputParameters validParams<SideSetsFromNormals>();

class SideSetsFromNormals : public AddSideSetsBase
{
public:
  SideSetsFromNormals(const InputParameters & parameters);

protected:
  virtual void modify() override;

  std::vector<BoundaryName> _boundary_names;

  std::vector<Point> _normals;
};

#endif /* SIDESETSFROMNORMALS_H */
