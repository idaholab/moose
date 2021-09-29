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

#include <string>

class Problem;
class Runner;

class Runner : public Executioner
{
public:
  struct Result
  {
    Result() : converged(true), _name("NO_NAME") {}
    Result(const std::string & name) : converged(true), _name(name) {}
    Result(const MooseObject * obj) : converged(true), _name(obj->name()) {}

    bool converged = true;
    std::string reason;
    std::map<std::string, Result> subs;

    std::string str(bool success_msg = false, const std::string & indent = "")
    {
      std::string s = indent + label(success_msg) + "\n";
      for (auto & entry : subs)
        s += entry.second.str(success_msg, indent + "  ") + "\n";
      return s;
    }

    void pass(const std::string & msg, bool overwrite = false)
    {
      mooseAssert(converged || overwrite,
                  "cannot override nonconverged executioner result with a passing one");
      ((void)(overwrite)); // avoid unused error due to assert
      reason = msg;
      converged = true;
    }
    void fail(const std::string & msg)
    {
      reason = msg;
      converged = false;
    }

    bool record(const std::string & name, const Result & r)
    {
      subs[name] = r;
      return r.convergedAll();
    }

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
    std::string label(bool success_msg)
    {
      return _name + "(" + (converged ? "success" : "FAIL") + ")" +
             ((success_msg || !converged) && !reason.empty() ? ": " + reason : "");
    }
    std::string _name;
  };

  Runner(const InputParameters & parameters);

  virtual ~Runner() {}

  static InputParameters validParams();

  Result run();

  virtual Result gogogadget() = 0;

  virtual void execute() override {}

  virtual bool lastSolveConverged() const override { return _result.convergedAll(); }

protected:
  ExecFlagType _begin_flag;
  ExecFlagType _end_flag;

private:
  Result _result;
};
