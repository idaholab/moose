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

  virtual void init();
  virtual void addVariables();
  virtual void addMooseObjects();

protected:
  FlowModel::EModelType _model_type;
  std::string _lm_name;
  std::vector<Real> _K;
  Real _scaling_factor;
  std::vector<Real> _scaling_factor_bcs;
};

#endif /* FLOWJUNCTION_H */
