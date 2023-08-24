//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputStream.h"

#include <string>

/**
 * Helper class that hands out input streams to a file.
 */
class FileInputStream : public InputStream
{
public:
  FileInputStream(const std::string & filename);

  virtual std::shared_ptr<std::istream> get() const override final;

  /**
   * @return The name of the managed file
   */
  const std::string & filename() const { return _filename; }

protected:
  /// The name of the file
  const std::string _filename;
};
