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

  XFEMElementPairLocator(MooseSharedPointer<XFEM> xfem, unsigned int interface_id);

  virtual ~XFEMElementPairLocator();

  virtual void reinit();

protected:

  MooseSharedPointer<XFEM> _xfem;

};

#endif // XFEMELEMENTPAIRLOCATOR_H
