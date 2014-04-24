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

#ifndef FILEOUTPUT_H
#define FILEOUTPUT_H

// MOOSE includes
#include "Output.h"

// Forward declerations
class FileOutput;

template<>
InputParameters validParams<FileOutput>();

/**
 * An outputter with filename support
 *
 * @see Exodus
 */
class FileOutput : public Output
{
public:

  /**
   * Class constructor
   */
  FileOutput(const std::string & name, InputParameters & parameters);

  /**
   * Class destructor
   */
  virtual ~FileOutput();

  /**
   * The filename for the output file
   * @return A string of output file including the extension
   */
  virtual std::string filename() = 0;

  /**
   * Performs the initial output, including the creation of the file base contains check
   */
  virtual void outputInitial();

  /**
   * Performs the output of a time step, including the creation of the file base contains check
   */
  virtual void outputStep();

  /**
   * Performs the final output, including the creation of the file base contains check
   */
  virtual void outputFinal();

  /**
   * Sets the file number manually.
   *
   * This method was implemented for the MultiApp system, particularly when reseting
   * an application and a new output file is desired after the reset.
   */
  void setFileNumber(unsigned int num);

  /**
   * Return the current file number for this outputter
   *
   * This method was implemented for the MultiApp system, particularly when reseting
   * an application and a new output file is desired after the reset.
   */
  unsigned int getFileNumber();

  /**
   * Returns the default output file base
   * @return The name of the input file with '_out' append to the end
   *
   * This method is static to allow for outside objects to call it, namely
   * CommonOutputAction::setRecoverFileBase().
   *
   * @see CommonOutputAction::setRecoverFileBase()
   */
  static std::string getOutputFileBase(MooseApp & app, std::string suffix = "_out");

protected:

  /**
   * Checks the filename for output
   * Checks the output against the 'output_if_base_contians' list
   * @return Returns true if the filename is valid for output
   */
  bool checkFilename();

  /// The base filename from the input paramaters
  std::string _file_base;

  /// A file number counter, initialized to 0 (this must be controlled by the child class, see Exodus)
  unsigned int & _file_num;

  /// Number of digits to pad the extensions
  unsigned int _padding;

  /// True if the file should be output (used for 'output_if_base_constains'
  bool _output_file;

  // OutputWarehouse needs access to _file_num for MultiApp ninja wizardry (see OutputWarehouse::merge)
  friend class OutputWarehouse;
};

#endif /* FILEOUTPUT_H */
