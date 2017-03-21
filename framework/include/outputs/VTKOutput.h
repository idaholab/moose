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

#ifndef VTKOUTPUT_H
#define VTKOUTPUT_H

// MOOSE includes
#include "BasicOutput.h"
#include "OversampleOutput.h"

// Forward declerations
class VTKOutput;

template <>
InputParameters validParams<VTKOutput>();

/**
 *
 */
class VTKOutput : public BasicOutput<OversampleOutput>
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
