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

#ifndef GETTRANSFERUSEROBJECT_H
#define GETTRANSFERUSEROBJECT_H

#include "GeneralUserObject.h"

// Forward Declarations
class GetTransferUserObject;

template <>
InputParameters validParams<GetTransferUserObject>();

class GetTransferUserObject : public GeneralUserObject
{
public:
  GetTransferUserObject(const InputParameters & parameters);
  virtual void execute() override;
  virtual void initialize() override;
  virtual void finalize() override;
};

#endif
