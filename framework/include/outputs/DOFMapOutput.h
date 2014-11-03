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

#ifndef DOFMAPOUTPUT_H
#define DOFMAPOUTPUT_H

// MOOSE includes
#include "FileOutput.h"

// Forward declarations
class DOFMapOutput;

template<>
InputParameters validParams<DOFMapOutput>();

/**
 * An output object for writing the DOF map of the system in a machine parsable format
 */
class DOFMapOutput :
  public FileOutput
{
public:
  DOFMapOutput(const std::string & name, InputParameters);
  virtual ~DOFMapOutput();

  /**
   * Initial setup function
   * Prints the DOF map information, this is done here so that the system information
   * is printed prior to any PETSc solve information
   */
  virtual void initialSetup();

  /**
   * Creates the output file name
   * Appends the user-supplied 'file_base' input parameter with a '.txt' extension
   * @return A string containing the output filename
   */
  virtual std::string filename();

  /**
   * Display the system information
   */
  void outputSystemInformation();

protected:

  /**
   * Write message to screen and/or file
   * @param message The desired message
   * @param indent True if multiapp indenting is desired
   */
  void write(std::string message);

  template<typename T>
  std::string join(const T & begin, const T & end, const char* const delim);

  /**
   * Apply indentation to newlines in the supplied stream
   * @param message Reference to the message being changed
   */
  void indentMessage(std::string & message);

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

  std::string _system_name;

  MooseMesh & _mesh;

private:

  /**
   * Add a message to the output streams
   * @param message The message to add to the output streams
   *
   * Any call to this method will write the supplied message to the screen and/or file,
   * following the same restrictions as outputStep and outputInitial.
   *
   * Calls to this method should be made via OutputWarehouse::mooseDOFMapOutput so that the
   * output stream buffer is cleaned up correctly. Thus, it is a private method.
   */
  void mooseConsoleOutput(const std::string & message);

  /// Reference to cached messages from calls to _console
  const std::ostringstream & _console_buffer;

  friend class OutputWarehouse;
};

#endif /* DOFMAPOUTPUT_H */
