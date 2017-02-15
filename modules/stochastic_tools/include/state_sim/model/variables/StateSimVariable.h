/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef STATESIMVARIABLE_H
#define STATESIMVARIABLE_H

#include "StateSimBase.h"
#include <string>

//Forward Declarations
class StateSimModel;

/**
 * A class to hold variables used in a StateSim model.  Types include (VT_DOUBLE, VT_STRING, VT_BOOL VT_DOUBLE_PTR)
 */
class StateSimVariable : public StateSimBase
{
public:
  /**
   * Constructors for a Real, string, and bool variables.
   * @param name - name of the item
   * @param scope - the scope of the variable.
   * @param value - the value of the variable;
   */
  StateSimVariable(StateSimModel & main_model, const std::string & name, VAR_SCOPE_ENUM scope, const Real & value);
  StateSimVariable(StateSimModel & main_model, const std::string & name, VAR_SCOPE_ENUM scope, const std::string & value);
  StateSimVariable(StateSimModel & main_model, const std::string & name, VAR_SCOPE_ENUM scope, const bool & value);

  /**
   * Constructors for Real reference variables, a link to an moose variable outside the StateSim model or simulation.
   * @param name - name of the item
   * @param value - the value of the variable;
   */
  StateSimVariable(StateSimModel & main_model, const std::string & name, Real & value);

  VAR_SCOPE_ENUM getScope()
  {
    return _var_scope;
  };
  VAR_TYPE getVarType()
  {
    return _var_type;
  };

  /**
   *  - set the value of the simulation variable for the correct type.
   * @param value - value to set the simulation variable to.
   */
  void setReal(const Real & value); // {_value = std::to_string(value);};
  void setTimespan(const TimespanH & value)
  {
    _value = std::to_string(value);
  };
  void setString(const std::string & value)
  {
    _value = value;
  };
  void setBool(const bool & value)
  {
    _value = value ? "true" : "false";
  };
  void setRealPtrVal(const Real & value)
  {
    *_value_ptr = value;
  };

  /**
   *  - get the value of the simulation variable for the correct type.
   * @return the value of the simuation variable.
   */
  Real getReal();
  std::string getString();
  bool getBool();
  Real getRealPtrVal();
  //std::string getTypeName();

  Real getReal() const;
  std::string getString() const;
  bool getBool() const;
  Real getRealPtrVal() const;
  std::string getTypeName() const;

  //TODO
  //getDerivedJSON
  //getJSON
  //deserializeDerived

protected:
  std::string _value;
  Real * _value_ptr;
  VAR_TYPE _var_type;

  //std::string geometry; //used for display
  VAR_SCOPE_ENUM _var_scope;
};

#endif
