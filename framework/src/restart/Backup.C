//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Backup.h"

#include "DataIO.h"

void
dataStore(std::ostream & stream, Backup *& backup, void * context)
{
  mooseAssert(backup->data, "Not set");
  dataStore(stream, *backup->data, context);
}

void
dataLoad(std::istream & stream, Backup *& backup, void * context)
{
  mooseAssert(backup->data, "Not set");
  dataLoad(stream, *backup->data, context);
}
