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

#ifndef ADDOUTPUTACTION_H
#define ADDOUTPUTACTION_H

// MOOSE includes
#include "MooseObjectAction.h"
#include "OutputWarehouse.h"
#include "MooseMesh.h"

// libMesh includes
#include "libmesh/equation_systems.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/mesh_function.h"

// Forward declerations
class AddOutputAction;
class OutputWarehouse;
class OutputBase;

template<>
InputParameters validParams<AddOutputAction>();

/**
 * Action for creating output objects
 */
class AddOutputAction: public MooseObjectAction
{
public:

  /**
   * Class constructor
   */
  AddOutputAction(const std::string & name, InputParameters params);

  /**
   * Creates the actual output object
   */
  virtual void act();

};

#endif //ADDOUTPUTACTION_H
