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
#include "PetscOutput.h"

/**
 * An outputter with filename support
 *
 * @see Exodus
 */
class FileOutput : public PetscOutput
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   */
  FileOutput(const InputParameters & parameters);

  /**
   * The filename for the output file
   * @return A string of output file including the extension, by default this returns _file_base
   */
  virtual std::string filename();

  /**
   * Sets the file base string if the 'file_base' parameter is not set
   *
   * @param file_base The new file base string
   */
  void setFileBase(const std::string & file_base);

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

protected:
  /**
   * Checks if the output method should be executed
   */
  virtual bool shouldOutput(const ExecFlagType & type) override;

  /**
   * Checks the filename for output
   * Checks the output against the 'output_if_base_contians' list
   * @return Returns true if the filename is valid for output
   */
  bool checkFilename();

  /**
   * Internal function that sets the file_base
   *
   * @param file_base The new file base string
   */
  virtual void setFileBaseInternal(const std::string & file_base);

  /// A file number counter, initialized to 0 (this must be controlled by the child class, see Exodus)
  unsigned int & _file_num;

  /// Number of digits to pad the extensions
  unsigned int _padding;

  /// Storage for 'output_if_base_contains'
  std::vector<std::string> _output_if_base_contains;

  /// The base filename from the input paramaters
  std::string _file_base;

private:
  // OutputWarehouse needs access to _file_num for MultiApp ninja wizardry (see
  // OutputWarehouse::merge)
  friend class OutputWarehouse;
};
