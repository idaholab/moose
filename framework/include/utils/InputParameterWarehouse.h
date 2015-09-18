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


struct MooseObjectName
{
  std::string tag;
  std::string name;

  MooseObjectName(const std::string & tag = std::string(), const std::string & name = std::string())
    {
      this->tag = tag;
      this->name = name;
    }


  bool operator==(const MooseObjectName & rhs) const
    {
      if ( (this->name == rhs.name) && (this->tag == rhs.tag || this->tag.empty() || rhs.tag.empty() ) )
           return true;
      return false;

    }

  bool operator!=(const MooseObjectName & rhs) const
    {
      return !( *this == rhs );

    }


  bool operator<(const MooseObjectName & rhs) const
    {
      return !std::lexicographical_compare(this->tag.begin(), this->tag.end(), rhs.tag.begin(), rhs.tag.end())
        && !std::lexicographical_compare(rhs.tag.begin(), rhs.tag.end(), this->tag.begin(), this->tag.end())
        && !std::lexicographical_compare(this->name.begin(), this->name.end(), rhs.name.begin(), rhs.name.end())
        && !std::lexicographical_compare(rhs.name.begin(), rhs.name.end(), this->name.begin(), this->name.end());

      //return !std::less<std::string>(lhs.tag, rhs.tag) && !std::less<std::string>(rhs.tag, lhs.tag) &&
      //       !std::less<std::string>(lhs.name, rhs.name) && !std::less<std::string>(rhs.name, lhs.name));

    }

  /*
  bool operator()(const MooseObjectName & lhs, const MooseObjectName & rhs)
    {
      return lhs == rhs;
    }
*/

};



typedef std::map<MooseObjectName, MooseSharedPointer<InputParameters> >::iterator InputParameterIterator;


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
  std::vector<std::map<MooseObjectName, MooseSharedPointer<InputParameters> > > _input_parameters;

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
  InputParameters & getInputParameters(const std::string & tag, const std::string & name, THREAD_ID tid = 0);

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
