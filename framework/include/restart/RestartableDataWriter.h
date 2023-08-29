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
 * Writer for restartable data, to be read by the RestartableDataReader.
 */
class RestartableDataWriter : public RestartableDataIO
{
public:
  RestartableDataWriter(MooseApp & app, RestartableDataMap & data);
  RestartableDataWriter(MooseApp & app, std::vector<RestartableDataMap> & data);

  /**
   * Writes the restartable data to header stream \p header_stream
   * and data stream \p data_stream
   */
  void write(std::ostream & header_stream, std::ostream & data_stream);
  /**
   * Writes the restartable data to the folder with base \p folder_base
   */
  std::vector<std::filesystem::path> write(const std::filesystem::path & folder_base);
};
