#ifndef FLOWJUNCTION_H
#define FLOWJUNCTION_H

#include <string>
#include "Junction.h"

class FlowJunction;

template<>
InputParameters validParams<FlowJunction>();

/**
 * Joint for flow
 */
class FlowJunction : public Junction
{
public:
  FlowJunction(const std::string & name, InputParameters params);
  virtual ~FlowJunction();

  virtual void addVariables();
  virtual void addMooseObjects();

protected:
  std::string _lm_name;
};

#endif /* FLOWJUNCTION_H */
