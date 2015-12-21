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

// MOOSE includes
#include "Warehouse.h"
#include "ParallelUniqueId.h"
#include "MooseObjectName.h"
#include "MooseTypes.h"

// Forward declarations
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

  typedef std::multimap<MooseObjectName, MooseSharedPointer<InputParameters> >::iterator InputParameterIterator;

  /**
   * Class constructor
   */
  InputParameterWarehouse();

  /**
   * Destruction
   */
  virtual ~InputParameterWarehouse();

  /**
   * This method is not valid, so it will produce an error
   *
   * The Warehouse::all() method returns raw pointers, which this warehouse does
   * not utilize. So, this method should not do anything.
   *
   * Use the begin() and end() iterator methods instead.
   */
  const std::vector<InputParameters *> & all() const;

  ///@{
  /**
   * Return a const reference to the InputParameters for the named object
   * @param tag The tag of the object (e.g., 'Kernel')
   * @param name The name of the parameters object, including the tag (name only input) or MooseObjectName object
   * @param tid The thread id
   * @return A const reference to the warehouse copy of the InputParameters
   */
  const InputParameters & getInputParametersObject(const std::string & name, THREAD_ID tid = 0) const;
  const InputParameters & getInputParametersObject(const std::string & tag, const std::string & name, THREAD_ID tid = 0 ) const;
  const InputParameters & getInputParametersObject(const MooseObjectName & object_name, THREAD_ID tid = 0 ) const;
  ///@{

private:

  /// Storage for the InputParameters objects
  std::vector<std::multimap<MooseObjectName, MooseSharedPointer<InputParameters> > > _input_parameters;

  /**
   * Method for adding a new InputParameters object
   * @param parameters The InputParameters object to copy and store in the warehouse
   * @return A reference to the warehouse copy of the InputParameters, this
   *         is what should be passed into the MooseObjects constructors.
   *
   * A new object is created from the old object because InputParameters objects
   * are generic until Factory::create() is called and the actual MooseObject
   * is created.
   *
   * This method is private, because only the factories that are creating objects should be
   * able to call this method.
   */
  InputParameters & addInputParameters(const std::string & name, InputParameters parameters, THREAD_ID tid = 0);

  ///@{
  /**
   * Return a reference to the InputParameters for the named object
   * @param tag The tag of the object (e.g., 'Kernel')
   * @param name The name of the parameters object, including the tag (name only input) or MooseObjectName object
   * @param tid The thread id
   * @return A reference to the warehouse copy of the InputParameters
   *
   * If you are using this method to access a writable reference to input parameters, this
   * will break the ability to control the parameters with the MOOSE control logic system.
   * Only change parameters if you know what you are doing. Hence, this is private for a reason.
   */
  InputParameters & getInputParameters(const std::string & name, THREAD_ID tid = 0) const;
  InputParameters & getInputParameters(const std::string & tag, const std::string & name, THREAD_ID tid = 0 ) const;
  InputParameters & getInputParameters(const MooseObjectName & object_name, THREAD_ID tid = 0 ) const;
  ///@{

  ///@{
  /**
   * Return iterators to the stored InputParameters object
   * @name tid The thread id
   * @return An iterator to the InputParameters object
   */
  InputParameterIterator begin(THREAD_ID tid = 0){ return _input_parameters[tid].begin(); }
  InputParameterIterator end(THREAD_ID tid = 0){ return _input_parameters[tid].end(); }
  ///@}

  friend class Factory;
  friend class ActionFactory;
  friend class Control;

  // RELAP-7 Control Logic (This will go away when the MOOSE system is created)
  friend class Component;
  friend class R7SetupOutputAction;
  friend class SolidMaterialProperties;
};

#endif // INPUTPARAMETERWAREHOUSE_H
