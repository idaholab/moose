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

#ifndef DYNSTEPPER_H
#define DYNSTEPPER_H

#include "TimeStepper.h"

class DynStepper;

template<>
InputParameters validParams<DynStepper>();



class DynStepper : public TimeStepper
{
public:
  DynStepper(const InputParameters & parameters);

  virtual Stepper * buildStepper() override;

private:
  const Real _constant_dt;
  const Real _growth_factor;
  Real & _last_dt;
};

class StepperNode {
public:
  std::string val;
  std::vector<StepperNode> args;

  std::string str(int level = 0) {
    std::stringstream ss;
    ss << std::string(level*4, ' ') << "Node '" << val << "' {\n";
    for (auto& sub : args) {
      ss << sub.str(level + 1);
    }
    ss << std::string(level*4, ' ') << "}\n";
    return ss.str();
  };

  template <class T>
  T get();

  template <class T>
  std::vector<T> getVec() {
    std::vector<T> vec;
    for (auto& n : args) {
      vec.push_back(n.get<T>());
    }
    return vec;
  }
};

template<>
int StepperNode::get<int>() {return std::atoi(val.c_str());}

template<>
double StepperNode::get<double>() {
  char* endptr;
  double v = strtod(val.c_str(), &endptr);
  if (*endptr) return 0;
  return v;
}

enum TokenType {
  LEFT_PAREN,
  RIGHT_PAREN,
  IDENT
};

struct StepperToken {
  TokenType type;
  std::string val;
  int index;
  std::string str() {
    std::stringstream ss;
    if (type == LEFT_PAREN) ss << "LEFT_PAREN";
    else if (type == RIGHT_PAREN) ss << "RIGHT_PAREN";
    else if (type == IDENT) ss << "IDENT(" + val + ")";
    return ss.str();
  }
};

std::vector<StepperToken> lexStepper(std::string& s) {
  std::vector<StepperToken> toks;
  int start = 0;
  bool nonblank = false;
  for (int i = 0; i < s.size(); i++) {
    char c = s[i];
    switch (c) {
    case '(':
      if (nonblank) {
        std::string val = s.substr(start, i - start);
        toks.push_back({IDENT, val, start});
        nonblank = false;
      }
      toks.push_back({LEFT_PAREN, "(", i});
      start = i+1;
      break;
    case ')':
      if (nonblank) {
        std::string val = s.substr(start, i - start);
        toks.push_back({IDENT, val, start});
        nonblank = false;
      }
      toks.push_back({RIGHT_PAREN, ")", i});
      start = i+1;
      break;
    case ' ':
      if (nonblank) {
        std::string val = s.substr(start, i - start);
        toks.push_back({IDENT, val, start});
        nonblank = false;
      }
      start = i+1;
      break;
    default:
      nonblank = true;
    }
  }
  return toks;
};

struct Err : std::exception {
  Err(std::string s) : _msg(s) { }
  const char* what() const noexcept {return _msg.c_str();}
  std::string _msg;
};

int parseStepperInner(std::vector<StepperToken> toks, int start, StepperNode& n) {
  if (start >= toks.size()) {
    std::stringstream ss;
    ss << "unmatched left paren at index " << toks[start-1].index;
    throw Err(ss.str());
  }

  int i = 0;
  for (i = start; i < toks.size(); i++) {
    switch (toks[i].type) {
    case LEFT_PAREN: {
      StepperNode sub;
      sub.val = "[list]";
      i = parseStepperInner(toks, i+1, sub);
      n.args.push_back(sub);
      break;
      }
    case RIGHT_PAREN:
      if (start == 0 && i < toks.size() - 1) {
        std::stringstream ss;
        ss << "unmatched right paren at index " << toks[i].index;
        throw Err(ss.str());
      }
      return i;
      break;
    case IDENT:
      n.args.push_back({toks[i].val, {}});
      break;
    }
  }
  if (start > 0) {
    std::stringstream ss;
    ss << "unmatched left paren at index " << toks[i-1].index;
    throw Err(ss.str());
  }
  return i;
};

StepperNode parseStepper(std::vector<StepperToken> toks) {
  StepperNode n;
  n.val = "[root]";
  parseStepperInner(toks, 0, n);
  return n;
}

Stepper* buildStepper(StepperNode n);

#endif /* DYNSTEPPER_H */
