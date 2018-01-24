/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
