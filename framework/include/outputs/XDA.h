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

#ifndef XDA_H
#define XDA_H

// MOOSE includes
#include "OversampleOutputter.h"

// Forward declearations
class XDA;

template<>
InputParameters validParams<XDA>();

/**
 * Class for output data to the XDAII format
 */
class XDA :
  public OversampleOutputter
{
public:

  /**
   * Class consturctor
   */
  XDA(const std::string & name, InputParameters);

protected:

  /**
   * Overload the OutputBase::output method, this is required for XDA
   * output due to the method utlized for outputing single/global parameters
   */
  virtual void output();

  /**
   * Returns the current filename, this method handles adding the timestep suffix
   * @return A string containg the current filename to be written
   */
  std::string filename();

  //@{
  /**
   * Individual component output is not supported for XDA/XDR
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

#endif /* XDA_H */
