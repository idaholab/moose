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

#ifndef USEROBJECTIO_H
#define USEROBJECTIO_H

#include "UserObjectWarehouse.h"

class UserObjectIO
{
public:
  UserObjectIO(UserObjectWarehouse & ud_wh);
  virtual ~UserObjectIO();

  virtual void write(const std::string & file_name);
  virtual void read(const std::string & file_name);

protected:
  UserObjectWarehouse & _userobject_wh;

  static const unsigned int file_version;
};

#endif /* USEROBJECTIO_H */
