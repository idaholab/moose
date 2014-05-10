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
#include "OversampleOutput.h"

// libMesh includes
#include "libmesh/vtk_io.h"

// Forward declerations
class VTKOutput;

template<>
InputParameters validParams<VTKOutput>();

/**
 *
 */
class VTKOutput :
  public OversampleOutput
{
public:

  /**
   * Class constructor
   * @param name Object name
   * @param parameters Object parameters
   */
  VTKOutput(const std::string & name, InputParameters & parameters);

  /**
   * Class destructor
   */
  virtual ~VTKOutput();

  /**
   * Creates the libMes::VTKOutputIO object for outputting the current timestep
   */
  virtual void outputSetup();


protected:

  /**
   * Perform the output of VTKOutput
   */
  virtual void output();

  /**
   * Return the file name with the *.vtk extension
   */
  std::string filename();

  //@{
  /**
   * Individual component output is not currently supported for VTKOutput
   */
  virtual void outputNodalVariables();
  virtual void outputElementalVariables();
  virtual void outputPostprocessors();
  virtual void outputVectorPostprocessors();
  virtual void outputScalarVariables();
  //@}

private:

  /// Pointer to libMesh::VTKIO object
  VTKIO * _vtk_io_ptr;

  /// Flag for using binary compression
  bool _binary;

};

#endif //VTKOUTPUT_H
