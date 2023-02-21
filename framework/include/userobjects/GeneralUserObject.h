//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "UserObject.h"
#include "MaterialPropertyInterface.h"
#include "TransientInterface.h"
#include "DependencyResolverInterface.h"
#include "ReporterInterface.h"

/* This class is here to combine the Postprocessor interface and the
 * base class Postprocessor object along with adding MooseObject to the inheritance tree*/
class GeneralUserObject : public UserObject,
                          public MaterialPropertyInterface,
                          public TransientInterface
{
public:
  static InputParameters validParams();

  GeneralUserObject(const InputParameters & parameters);

  ///@{
  /**
   * This method is not used and should not be used in a custom GeneralUserObject.
   */
  virtual void threadJoin(const UserObject &) override;
  virtual void subdomainSetup() override;
  ///@}
};
