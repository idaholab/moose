//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef XFEMELEMENTPAIRLOCATOR_H
#define XFEMELEMENTPAIRLOCATOR_H

#include "ElementPairLocator.h"
#include "XFEM.h"

class XFEMElementPairLocator : public ElementPairLocator
{
public:
  XFEMElementPairLocator(MooseSharedPointer<XFEM> xfem,
                         unsigned int interface_id,
                         bool use_displaced_mesh = false);
  virtual void reinit();
  virtual void update();

protected:
  MooseSharedPointer<XFEM> _xfem;
  bool _use_displaced_mesh;
};

#endif // XFEMELEMENTPAIRLOCATOR_H
