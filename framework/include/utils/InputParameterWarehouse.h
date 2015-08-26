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

// Forward declarations
class InputParameters;
class MooseApp;

/**
 * A storage container for the name of parameters to be controlled
 */
struct InputParametersContainer
{
  /// The "system" name (e.g., Kernels)
  std::string system;

  /// The user-defined tag assigned to the object
  std::string tag;

  /// The "object" name (generally a sub block in an input file)
  std::string object;

  /// The name of the parameter to be controlled
  std::string param;

  /// Input file syntax provided by creating Action
  std::string syntax;

  /// Pointer to the parameters object
  MooseSharedPointer<InputParameters> parameters;
};


typedef std::vector<InputParametersContainer>::iterator InputParameterIterator;

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
   * This method is not valid, so it will produce an error
   *
   * The Warehouse::all() method returns raw pointers, which this warehouse does
   * not utilize. So, this method should not do anything.
   *
   * Use the begin() and end() iterator methods instead.
   */
  const std::vector<InputParameters *> & all() const;


private:

  /// Storage for the InputParameters objects
  std::vector<std::vector<InputParametersContainer> > _input_parameters;

  ///@{
  /// Maps from the name of the object to the pointer index in _input_parameters
  std::vector<std::map<std::string, unsigned int> > _system_to_index;
  std::vector<std::map<std::string, unsigned int> > _syntax_to_index;
  std::vector<std::map<std::string, unsigned int> > _tag_to_index;
  ///@}

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
  InputParameters & addInputParameters(std::string name, InputParameters parameters, THREAD_ID tid = 0);

  /**
   * Return a reference to the InputParameters for the named object
   * @name long_name The full name of the object for which parameters are desired
   * @name tid The thread id
   * @return A const reference to the warehouse copy of the InputParameters
   *
   * Note, the long_name can be supplied in two forms:
   *   SystemBase::object_name
   *   InputSyntax/object_name
   *
   * If you are using this method to access a writable reference to input parameters, this
   * will break the ability to control the parameters with the MOOSE control logic system.
   * Only change parameters if you know what you are doing. Hence, this is private for a reason.
   */
  InputParameters & getInputParameters(const std::string & long_name, THREAD_ID tid = 0);

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
  friend class ControlInterface;

  // RELAP-7 Control Logic (This will go away when the MOOSE system is created)
  friend class Component;
  friend class R7SetupOutputAction;
  friend class SolidMaterialProperties;
};

#endif // INPUTPARAMETERWAREHOUSE_H
