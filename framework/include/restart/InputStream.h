//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <memory>
#include <istream>
#include <vector>
#include <filesystem>
#include <optional>

/**
 * Helper class that hands out input streams to an underlying, managed
 * stream of arbitrary type.
 */
class InputStream
{
public:
  virtual ~InputStream();

  /**
   * Gets an input stream to the underlying stream.
   *
   * This is returned as a shared_ptr so that this class can tell
   * whether or not another object is still using the stream.
   */
  virtual std::shared_ptr<std::istream> get() const = 0;

  /**
   * Whether or not anything is still using this stream.
   *
   * This is checked by seeing whether or not the use count
   * of any of the streams handed out by get() is > 1 (with the
   * only use count being the one stored in _shared_streams)
   */
  bool inUse() const;

  /**
   * Gets the underlying filename, if any
   */
  virtual std::optional<std::filesystem::path> getFilename() const;

protected:
  /**
   * Internal method to be called by derived classes to add a shared
   * stream to _shared_streams (so that it can be tracked).
   *
   * Should be called in the overridden get() by derived classes.
   */
  void addSharedStream(const std::weak_ptr<std::istream> stream) const;

private:
  /// The streams that have been handed out by get()
  mutable std::vector<std::weak_ptr<std::istream>> _shared_streams;
};
