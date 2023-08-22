//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RestartableDataIO.h"

/**
 * Writer for restartable data.
 */
class RestartableDataWriter : public RestartableDataIO
{
public:
  RestartableDataWriter(MooseApp & app, RestartableDataMap & data);
  RestartableDataWriter(MooseApp & app, std::vector<RestartableDataMap> & data);

  /**
   * Writes the restartable data to the output stream \p stream
   */
  void write(std::ostream & stream);
  /**
   * Writes the restartable data to the file at \p file_name
   */
  void write(const std::string & file_name);
};
