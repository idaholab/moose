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
   * Perform the output of VTK
   */
  virtual void output();

  /**
   * Return the file name with the *.vtk extension
   */
  std::string filename();

  //@{
  /**
   * Individual component output is not currently supported for VTK
   */
  virtual void outputNodalVariables();
  virtual void outputElementalVariables();
  virtual void outputPostprocessors();
  virtual void outputScalarVariables();
  //@}

private:

  /// Pointer to libMesh::VTKIO object
  VTKIO * _vtk_io_ptr;

  /// Flag for using binary compression
  bool _binary;

};

#endif //VTK_H
