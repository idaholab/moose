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

#ifndef NODALNORMALSCORNER_H
#define NODALNORMALSCORNER_H

#include "SideUserObject.h"

class NodalNormalsCorner;
class AuxiliarySystem;

template<>
InputParameters validParams<NodalNormalsCorner>();

/**
 *
 */
class NodalNormalsCorner : public SideUserObject
{
public:
  NodalNormalsCorner(const std::string & name, InputParameters parameters);
  virtual ~NodalNormalsCorner();

  virtual void initialize();
  virtual void destroy();
  virtual void finalize();
  virtual void execute();
  virtual void threadJoin(const UserObject & uo);

protected:
  AuxiliarySystem & _aux;
  BoundaryID _corner_boundary_id;
};


#endif /* NODALNORMALSCORNER_H */
