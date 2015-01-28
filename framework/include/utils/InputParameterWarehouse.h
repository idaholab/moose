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

#ifndef INPUTPARAMETERWAREHOUSE_H
#define INPUTPARAMETERWAREHOUSE_H

#include "Warehouse.h"

class InputParameters;

/**
 * Storage container for all InputParamter objects.
 *
 * This object is responsible for InputParameter objects, all MooseObjects should
 * contain a reference to the parameters object stored here.
 *
 */
class InputParameterWarehouse : public Warehouse<InputParameters>
{
public:

  /**
   * Class constructor
   */
  InputParameterWarehouse();

  /**
   * Destruction
   */
  virtual ~InputParameterWarehouse();


  /**
   * Method for adding a new InputParameters object
   *
   * A new object is created from the object passed in...
   *    1. InputParameters objects are considered generic,
   *       only when Factory.create() is called to they become tracked
   *
   */
  const InputParameters & addInputParameters(const InputParameters & params);

  const InputParameters & getInputParameters(const std::string & name);

private:
  std::map<std::string, MooseSharedPointer<InputParameters> > _name_to_shared_pointer;

};

#endif // INPUTPARAMETERWAREHOUSE_H
