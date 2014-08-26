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

#ifndef OUTPUTINTERFACE_H
#define OUTPUTINTERFACE_H

// MOOSE includes
#include "InputParameters.h"

// Forward declerations
class OutputInterface;
class OutputWarehouse;

template<>
InputParameters validParams<OutputInterface>();

/**
 * A class to provide an common interface to objects requiring "outputs" option
 *
 * The 'outputs' option, when set restricts the output of the variable(s) associated with
 * this object to only occur on output objects listed.
 */
class OutputInterface
{
public:
  OutputInterface(const std::string & name, InputParameters parameters);
  OutputInterface(const std::string & name, InputParameters parameters, std::string variable_name);
  OutputInterface(const std::string & name, InputParameters parameters, std::vector<std::string> variable_names);

private:

  MooseApp & _oi_moose_app;

  OutputWarehouse & _oi_output_warehouse;

  std::set<OutputName> _oi_outputs;
};

#endif // OUTPUTINTERFACE_H
