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

#ifndef BCWAREHOUSE_H
#define BCWAREHOUSE_H

#include <map>
#include <vector>

#include "IntegratedBC.h"
#include "NodalBC.h"


class BCWarehouse
{
public:
  BCWarehouse();
  virtual ~BCWarehouse();

  void addBC(unsigned int boundary_id, IntegratedBC *bc);
  void addNodalBC(unsigned int boundary_id, NodalBC *bc);

  /**
   * Get boundary conditions on a specified boundary id
   */
  std::vector<IntegratedBC *> & getBCs(unsigned int boundary_id);
  /**
   * Get nodal boundary conditions on a specified boundary id
   */
  std::vector<NodalBC *> & getNodalBCs(unsigned int boundary_id);

  void activeBoundaries(std::set<short> & set_buffer) const;

protected:
  std::map<unsigned int, std::vector<IntegratedBC *> > _bcs;
  std::map<unsigned int, std::vector<NodalBC *> > _nodal_bcs;
};

#endif // BCWAREHOUSE_H
