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

#ifdef LIBMESH_HAVE_DTK

#ifndef MOABTRANSFER_H
#define MOABTRANSFER_H

#include "Transfer.h"

class MoabTransfer;

template<>
InputParameters validParams<MoabTransfer>();

class MoabTransfer :
  public Transfer
{
public:
  MoabTransfer(const std::string & name, InputParameters parameters);
  virtual ~MoabTransfer() {}

  virtual void execute();

protected:
  MooseEnum _direction;
};

#endif /* MOABTRANSFER_H */

#endif //LIBMESH_HAVE_DTK
