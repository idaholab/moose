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
