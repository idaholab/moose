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

#ifndef MESHSIDESET_H
#define MESHSIDESET_H

#include "MeshModifier.h"

class MeshSideSet;

template <>
InputParameters validParams<MeshSideSet>();

/**
 * Add lower dimensional elements along the faces contained in a side set
 */
class MeshSideSet : public MeshModifier
{
public:
  MeshSideSet(const InputParameters & parameters);

  virtual void modify() override;

protected:
  /// Block ID to assign to the region
  const SubdomainID _block_id;
};

#endif // MESHSIDESET_H
