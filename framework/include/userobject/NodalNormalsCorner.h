//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NODALNORMALSCORNER_H
#define NODALNORMALSCORNER_H

#include "SideUserObject.h"

class NodalNormalsCorner;
class AuxiliarySystem;

template <>
InputParameters validParams<NodalNormalsCorner>();

/**
 *
 */
class NodalNormalsCorner : public SideUserObject
{
public:
  NodalNormalsCorner(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void finalize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & uo) override;

protected:
  AuxiliarySystem & _aux;
  BoundaryID _corner_boundary_id;
};

#endif /* NODALNORMALSCORNER_H */
