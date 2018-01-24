//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef VTKOUTPUT_H
#define VTKOUTPUT_H

// MOOSE includes
#include "OversampleOutput.h"

// Forward declerations
class VTKOutput;

template <>
InputParameters validParams<VTKOutput>();

/**
 *
 */
class VTKOutput : public OversampleOutput
{
public:
  /**
   * Class constructor
   * @param parameters Object parameters
   */
  VTKOutput(const InputParameters & parameters);

protected:
  /**
   * Perform the output of VTKOutput
   */
  virtual void output(const ExecFlagType & type) override;

  /**
   * Return the file name with the *.vtk extension
   */
  virtual std::string filename() override;

private:
  /// Flag for using binary compression
  bool _binary;
};

#endif // VTKOUTPUT_H
