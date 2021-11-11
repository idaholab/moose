//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Executioner.h"
#include "ExecutorInterface.h"

#include <string>

class Problem;
class Executor;

/// The Executor class directs the execution flow of simulations.  It manages
/// outputting, time stepping, mesh adaptivity, solve sequencing, multiapp
/// execution, etc.  Users can compose executor objects in an input file using
/// the same pattern as with mesh generators.  Subclass here and implement your
/// own run function if you need special or fancy functionality that
/// isn't supported by the default executors available.
class Executor : public Executioner, public ExecutorInterface
{
public:
  /// This object tracks the success/failure state of the executor system as
  /// execution proceeds in a simulation.  Because executors can be composed into
  /// trees, result objects are correspondingly composed into trees to track
  /// simulation progress.  Result objects should generally be created by
  /// executors calling the Result::newResult() function rather than by using
  /// the Result constructor directly.
  struct Result
  {
    Result() : converged(true), _name("NO_NAME") {}
    Result(const std::string & name) : converged(true), _name(name) {}
    Result(const MooseObject * obj) : converged(true), _name(obj->name()) {}

    /// whether or not a executor ran its code successfully - only reports
    /// results from the executor itself.  If a sub/internal executor of a executor
    /// fails, that sub-executor's result object will have converged=false, while
    /// the parent may still have converged=truee. Users should use convergedAll
    /// to recursively determine if there was any descendant executor failure.
    bool converged = true;

    /// Optional message detailing why an executor passed or failed (i.e. failed to converge).
    std::string reason;

    /// Maps a name/label of a executor's internal/sub executors to the result
    /// object returned by running each of those internal/sub executors.  This
    /// member should generally not be accessed directly.  It should generally
    /// be populated through the record function.  Info contained in these
    /// results will be included in printouts from the str function.
    std::map<std::string, Result> subs;

    /// Prints a full recursive output of this result object - including all
    /// descendant's results.  If success_msg is true, then all result output
    /// that contains a message will be printed even if it converged/passed.
    /// Otherwise, messages will only be printed for unconverged/failed results.
    std::string
    str(bool success_msg = false, const std::string & indent = "", const std::string & subname = "")
    {
      std::string s = indent + label(success_msg, subname) + "\n";
      for (auto & entry : subs)
        s += entry.second.str(success_msg, indent + "    ", entry.first);
      return s;
    }

    /// Marks the result as passing/converged with the given msg text
    /// describing detail about how things ran.  A result object is in this
    /// state by default, so it is not necessary to call this function for
    /// converged/passing scenarios.
    void pass(const std::string & msg, bool overwrite = false)
    {
      mooseAssert(converged || overwrite,
                  "cannot override nonconverged executioner result with a passing one");
      ((void)(overwrite)); // avoid unused error due to assert
      reason = msg;
      converged = true;
    }

    /// Marks the result as failing/unconverged with the given msg text
    /// describing detail about how things ran.
    void fail(const std::string & msg)
    {
      reason = msg;
      converged = false;
    }

    /// Records results from sub/internal executors in a executor's result.  When
    /// child-executors return a result object following their execution, this
    /// function should be called to add that info into the result hierarchy.
    /// If the child executor was identified by a label/text from the input file
    /// (e.g. via sub_solve1=foo_executor) - then "name" should be "sub_solve1".
    bool record(const std::string & name, const Result & r)
    {
      subs[name] = r;
      return r.convergedAll();
    }

    /// Returns false if any single executor in the current hierarchy of results
    /// (i.e. including all child results accumulated recursively via record)
    /// had a failed/unconverged return state.  Returns true otherwise.  This
    /// is how convergence should generally be checked/tracked by executors -
    /// rather than accessing e.g. the converged member directly.
    bool convergedAll() const
    {
      if (!converged)
        return false;
      for (auto & entry : subs)
        if (!entry.second.convergedAll())
          return false;
      return true;
    }

  private:
    std::string label(bool success_msg, const std::string & subname = "")
    {
      std::string state_str =
          success_msg || !converged ? (std::string("(") + (converged ? "pass" : "FAIL") + ")") : "";
      return subname + (subname.empty() ? "" : ":") + _name + state_str +
             ((success_msg || !converged) && !reason.empty() ? ": " + reason : "");
    }
    std::string _name;
  };

  Executor(const InputParameters & parameters);

  virtual ~Executor() {}

  static InputParameters validParams();

  /// This is the main function for executors - this is how executors should
  /// invoke child/sub executors - by calling their exec function.
  Result exec();

  virtual void execute() override final {}

  /// Executors need to return a Result object describing how execution went -
  /// rather than constructing Result objects directly, the newResult function
  /// should be called to generate new objects.  *DO NOT* catch the result by
  /// value - if you do, MOOSE cannot track result state for restart capability.
  /// You must catch result values from this function by reference.
  Result & newResult()
  {
    _result = Result(this);
    return _result;
  }

  /// Whether the executor and all its sub-executors passed / converged
  virtual bool lastSolveConverged() const override { return _result.convergedAll(); }

protected:
  /// This function contains the primary execution implementation for a
  /// executor.  Custom executor behavior should be localized to this function.  If
  /// you are writing a executor - this is basically where you should put all your
  /// code.
  virtual Result run() = 0;

  /// The execute-on flag to associate with the beginning of this executor's execution.
  /// This allows the framework and users to trigger other object execution by
  /// associating other objects with this flag.
  ExecFlagType _begin_flag;
  /// The execute-on flag to associate with the end of this executor's execution.
  /// This allows the framework and users to trigger other object execution by
  /// associating other objects with this flag.
  ExecFlagType _end_flag;

private:
  /// Stores the result representing the outcome from the run function.  It is
  /// a local member variable here to facilitate restart capability.
  Result _result;
};
