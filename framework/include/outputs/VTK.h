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

#ifndef VTK_H
#define VTK_H

// MOOSE includes
#include "OversampleOutputter.h"
#include "FileOutputInterface.h"

// libMesh includes
#include "libmesh/vtk_io.h"

// Forward declerations
class VTKOutputter;

template<>
InputParameters validParams<VTKOutputter>();

/**
 *
 */
class VTKOutputter :
  public OversampleOutputter,
  public FileOutputInterface
{
public:

  /**
   * Class constructor
   * @param name Object name
   * @param parameters Object parameters
   */
  VTKOutputter(const std::string & name, InputParameters & parameters);

  /**
   * Class destructor
   */
  virtual ~VTKOutputter();

  /**
   * Creates the libMes::VTKIO object for outputting the current timestep
   */
  virtual void outputSetup();

  
protected:
  
  /**
   * Outputs nodal, nonlinear variables
   */
  virtual void outputNodalVariables();

  /**
   * Outputs elemental, nonlinear variables
   * This does not do anything for VTK files currently, the libMesh VTK API needs some work
   * to function in this manner
   */
  virtual void outputElementalVariables();

  /**
   * Writes postprocessor values to global output parameters
   */
  virtual void outputPostprocessors();

  /**
   * Writes scalar AuxVariables to global output parameters
   */
  virtual void outputScalarVariables();

  /**
   * Return the file name with the *.vtk extension
   */
  std::string filename();

private:

  /// Pointer to libMesh::VTKIO object
  VTKIO * _vtk_io_ptr;

  /// Number of digits to pad the output files with
  unsigned int _padding;

  /// Flag for using binary compression
  bool _binary;
  
};

#endif //VTK_H
