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

#ifndef GMVOUTPUTTER_H
#define GMVOUTPUTTER_H

// MOOSE includes
#include "OversampleOutputter.h"
#include "FileOutputInterface.h"

// Forward declearations
class GMVOutputter;

template<>
InputParameters validParams<GMVOutputter>();

/**
 * Class for output data to the GMVOutputterII format
 */
class GMVOutputter :
  public OversampleOutputter,
  public FileOutputInterface
{
public:

  /**
   * Class consturctor
   */
  GMVOutputter(const std::string & name, InputParameters);

protected:

  /**
   * Overload the OutputBase::output method, this is required for GMVOutputter
   * output due to the method utlized for outputing
   */
  virtual void output();

  /**
   * Returns the current filename, this method handles adding the timestep suffix
   * @return A string containg the current filename to be written
   */
  std::string filename();

  //@{
  /**
   * Individual component output is not supported for GMVOutputter
   */
  virtual void outputNodalVariables();
  virtual void outputElementalVariables();
  virtual void outputPostprocessors();
  virtual void outputScalarVariables();
  //@}

private:

  /// Flag for binary output
  bool _binary;
};

#endif /* GMVOUTPUTTER_H */
