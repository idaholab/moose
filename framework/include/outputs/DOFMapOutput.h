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
#include "FileOutput.h"

class MooseMesh;

/**
 * An output object for writing the DOF map of the system in a machine parsable format
 */
class DOFMapOutput : public FileOutput
{
public:
  static InputParameters validParams();

  DOFMapOutput(const InputParameters & parameters);

  /**
   * Creates the output file name
   * Appends the user-supplied 'file_base' input parameter with a '.txt' extension
   * @return A string containing the output filename
   */
  virtual std::string filename() override;

  /**
   * Write the DOF mapt
   */
  void output(const ExecFlagType & type) override;

protected:
  /**
   * A helper method for joining items with a delimeter
   * @param begin Beginning iterator
   * @param end Ending iterator
   * @param delim The delimiter character to insert
   */
  template <typename T>
  std::string join(const T & begin, const T & end, const char * const delim);

  /**
   * Write the file stream to the file
   * @param append Toggle for appending the file
   *
   * This helper function writes the _file_output_stream to the file and clears the
   * stream, by default the file is appended. This does nothing if 'output_file' is
   * false.
   */
  void writeStreamToFile(bool append = true);

  /**
   * Try to demangle type name
   */
  std::string demangle(const std::string & name);

  /// Flag for controlling outputting console information to a file
  bool _write_file;

  /// Flag for controlling outputting console information to screen
  bool _write_screen;

  /// Stream for storing information to be written to a file
  std::stringstream _file_output_stream;

  /// The name of the system to extract DOF information
  std::string _system_name;

  /// Reference to the mesh object
  MooseMesh & _mesh;
};
