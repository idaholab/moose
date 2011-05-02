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

#ifndef MOOSEINIT_H
#define MOOSEINIT_H

#include "libmesh.h"
#include "getpot.h"

/**
 * Initialization object for any MOOSE-based application
 *
 * This object must be created in the main() of any MOOSE-based application so
 * everything is properly initialized and finalized.
 */
class MooseInit : public LibMeshInit
{
public:
  MooseInit(int argc, char *argv[]);
  virtual ~MooseInit();

protected:
};

namespace Moose
{

extern GetPot *command_line;

} // namespace Moose

#endif /* MOOSEINIT_H */
