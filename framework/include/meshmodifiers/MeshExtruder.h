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

#ifndef MESHEXTRUDER_H
#define MESHEXTRUDER_H

#include "MeshModifier.h"

class MeshExtruder;

template<>
InputParameters validParams<MeshExtruder>();

class MeshExtruder : public MeshModifier
{
public:
  MeshExtruder(const std::string & name, InputParameters parameters);

  virtual ~MeshExtruder();

  virtual void modify();

protected:
  const unsigned int _num_layers;
  const RealVectorValue _extrusion_vector;

private:
  void changeID(const std::vector<BoundaryName> & names, BoundaryID old_id);
};

#endif /* MESHEXTRUDER_H */
