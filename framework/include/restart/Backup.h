//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <sstream>
#include <memory>

/**
 * Helper class to hold streams for Backup and Restore operations.
 */
struct Backup
{
  std::unique_ptr<std::stringstream> header = std::make_unique<std::stringstream>();
  std::unique_ptr<std::stringstream> data = std::make_unique<std::stringstream>();
};

void dataStore(std::ostream & stream, Backup & backup, void * context);
void dataLoad(std::istream & stream, Backup & backup, void * context);
void dataStore(std::ostream & stream, std::unique_ptr<Backup> & backup, void * context);
void dataLoad(std::istream & stream, std::unique_ptr<Backup> & backup, void * context);
