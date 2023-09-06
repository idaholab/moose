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
dataStore(std::ostream & stream, Backup & backup, void * context)
{
  mooseAssert(backup.header, "Not set");
  mooseAssert(backup.data, "Not set");

  dataStore(stream, *backup.header, context);
  dataStore(stream, *backup.data, context);
}

void
dataLoad(std::istream & stream, Backup & backup, void * context)
{
  mooseAssert(backup.header, "Not set");
  mooseAssert(backup.data, "Not set");

  dataLoad(stream, *backup.header, context);
  dataLoad(stream, *backup.data, context);
}

void
dataStore(std::ostream & stream, std::unique_ptr<Backup> & backup, void * context)
{
  bool has_value = backup != nullptr;
  dataStore(stream, has_value, nullptr);
  if (has_value)
    dataStore(stream, *backup, context);
}

void
dataLoad(std::istream & stream, std::unique_ptr<Backup> & backup, void * context)
{
  bool has_value;
  dataLoad(stream, has_value, nullptr);
  if (has_value)
  {
    backup = std::make_unique<Backup>();
    dataLoad(stream, *backup, context);
  }
}
