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

#include <sstream>

/**
 * Helper class that hands out input streams to a stringstream.
 */
class StringInputStream : public InputStream
{
public:
  StringInputStream(std::unique_ptr<std::stringstream> stream);

  virtual std::shared_ptr<std::istream> get() const override final;

  /**
   * Releases the owned stringstream.
   *
   * Will error if an object still has access to a stream obtained via get().
   *
   * Leaves the owned stream in an undefined state.
   */
  std::unique_ptr<std::stringstream> release();

private:
  /// The underlying stringstream
  std::unique_ptr<std::stringstream> _stream;
};
