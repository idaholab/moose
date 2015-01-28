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

typedef std::map<std::string, MooseSharedPointer<InputParameters> >::const_iterator InputParameterIterator;

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
   * @param parameters The InputParameters object to copy and store in the warehouse
   * @return A const reference to the warehouse copy of the InputParameters, this
   *         is what should be passed into the MooseObjects constructors.
   *
   * A new object is created from the old object because InputParameters objects
   * are generic until Factory""create() is called and the actual MooseObject
   * is created.
   *
   */
  const InputParameters & addInputParameters(const InputParameters & parameters);

  /**
   * Return a reference to the InputParameters for the named object
   * @name The full name of the object for which parameters are desired
   * @return A const reference to the warehouse copy of the InputParameters
   */
  const InputParameters & getInputParameters(const std::string & name);

  ///@{
  /**
   * Iterators to the InputParameter objects
   * @return Const iterator to the map that stores the parameter objects
   */
  InputParameterIterator begin();
  InputParameterIterator end();
  ///@}

  /**
   * This method is not valid, so it will produce an error
   *
   * The Warehouse::all() method returns raw pointers, which this warehouse does
   * not utilize. So, this method should not do anything.
   *
   * Use the begin() and end() iterator methods instead.
   */
  const std::vector<InputParameters *> & all();

private:

  /// Name to pointer map for easy access to the pointers
  std::map<std::string, MooseSharedPointer<InputParameters> > _name_to_shared_pointer;

};

#endif // INPUTPARAMETERWAREHOUSE_H
