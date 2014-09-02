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

#ifndef POSTPROCESSOR_H
#define POSTPROCESSOR_H

#include <string>

// MOOSE includes
#include "InputParameters.h"
#include "Restartable.h"
#include "OutputInterface.h"

// libMesh
#include "libmesh/parallel.h"

class Postprocessor;

template<>
InputParameters validParams<Postprocessor>();


class Postprocessor : public OutputInterface
{
public:
  Postprocessor(const std::string & name, InputParameters parameters);

  virtual ~Postprocessor(){ }

  /**
   * This will get called to actually grab the final value the postprocessor has calculated.
   */
  virtual PostprocessorValue getValue() = 0;

  /**
   * Returns the name of the Postprocessor.
   */
  std::string PPName() { return _pp_name; }

protected:
  std::string _pp_name;
};

#endif
