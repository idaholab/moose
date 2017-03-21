/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef STATESIMBASE_H
#define STATESIMBASE_H

#include "MooseRandom.h"
#include <string>
#include <stdexcept>

typedef Real TimespanH; //hours

// const TimespanH YEAR_TIME(8760);
// const TimespanH DAY_TIME(24);
// const TimespanH HOUR_TIME(1);
// const TimespanH ZERO_TIME(0);

enum class STATE_TYPE_ENUM
{
  ST_START, //a starting state for the parent diagram
  ST_STANDARD, //a Standard state
  ST_KEYSTATE, //A key state that if in this state when simulation ends, data and path is recorded
  ST_TERMINAL //A state that if reached terminates the simulation.
};

enum class ACTION_TYPE_ENUM
{
  AT_TRANSITION, //move from one state to another
  AT_RUN_CODE//, //run a user defined fuction
  //AT_CUSTOM_STATE_SHIFT,
  //AT_JUMP_TO_TIME
};

enum class EVENT_TYPE_ENUM //EnEventType
{
  ET_FAIL_RATE, //This item is time based event where a failure rate is sampled for the next time
  ET_EXTERNAL,  //This event is triggered through a value set externaly from the StateSim
  ET_STATE_CNG, //executes when a diffent desired state\s are executed
  ET_LOGIC_EVAL, //executes when the logic tree evaluaction criteria is met
  ET_TIMER,  //executes when a defined time critera is met.
  ET_CODE_EVAL //Code is evaluated to determine if the event is executed
  // etVarCond, //executes when the value of a varible meets a condition
  // etNormalDist //time event following a normal distribution.
};

enum class DIAGRAM_TYPE_ENUM
{
  DT_COMPONENT, //Component diagram, can evaluate, limited to one current state
  DT_SYSTEM, //System diagram, can evaluate, limited to one current state
  DT_PLANT, //Plant responce diagram
  DT_OTHER
};

enum class GATE_TYPE_ENUM
{
  GT_AND, GT_OR, GT_NOT, GT_NOFM
};

enum class VAR_SCOPE_ENUM
{
  VS_LOCAL,
  VS_GLOBAL,
  VS_MOOSE //tied to a real from a different moose app.
  //todo VS_EXTERNAL //results from running an external application
};

enum class VAR_TYPE
{
  VT_DOUBLE,
  VT_STRING,
  VT_BOOL,
  VT_DOUBLE_PTR
};

enum class STATESIM_TYPE
{
  ACTION = 0,
  DIAGRAM = 1,
  EVENT = 2,
  LOGIC_GATE = 3,
  STATE = 4,
  VARIABLE = 5,
  FIRST = ACTION,
  LAST = VARIABLE
};

/**
 * Simple class to store the path of state movement and the reason for that movement.
 */
struct StateSimIdAndDesc
{
public:
  /**
   * This is the main construter for the user.
   * @param in_id - item id
   * @param in_desc - description.
   */
  StateSimIdAndDesc(int in_id, std::string in_desc)
  : id(in_id),
    desc(in_desc)
  {};

  int id;
  std::string desc;
};

/**
 * StateSimBase is the root of all the SateSim objects, each has a name, id (auto), desc.
 */
class StateSimBase
{
public:
  static const TimespanH _DAY_TIME;
  constexpr static TimespanH YEAR_TIME = 8760;
  constexpr static TimespanH DAY_TIME = 24;
  constexpr static TimespanH HOUR_TIME = 1;
  constexpr static TimespanH ZERO_TIME = 0;

  /**
   * Default constructor all derived objects use
   * @param name - the name of the object
   */
  StateSimBase(const int & bit_id, const std::string & name);
  virtual ~StateSimBase() = 0; //needed to make the class abstract, there are no other virtual functions
  std::string name() const {return _name;};
  int id() const {return _id;};
  const std::string desc() const;
  void setDesc(std::string desc) {_desc = desc;};
  /**
   * setProcessed - set the processed flag for the object to a new value.  Used when going through all the objecs in a list
   * @param value - the name of the object
   */
  void setProcessed(bool value) {_processed = value;};
  /**
   * processed - the value of the processed flag for the object.  Used when going through all the objecs in a list
   * @return the processed flag
   */
  bool processed() const {return _processed;};

  /**
   * nextRandom - Used for geting a random # so random # retreval will use the same method and this method can be changed.
   * @return a random number
   */
  Real nextRandom() const {return MooseRandom::rand();};

  //todo
  //virtual std::string getJSON
  //virtual bool deserializeJSON
  //virtual bool deserializeDerived
  //virtual bool loadObjLinks //overrride this for the items that need to load links after initial deserializeDerived of each list is done.

protected:
  int _id; //ids are local only set at construction, to be used for lookups where names can't be used like bitsets
  std::string _name;
  std::string _desc;
  bool _processed;
};

#endif
