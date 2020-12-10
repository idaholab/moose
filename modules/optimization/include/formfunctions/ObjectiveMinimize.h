#pragma once

#include "FormFunction.h"

class ObjectiveMinimize : public FormFunction
{
public:
  static InputParameters validParams();
  ObjectiveMinimize(const InputParameters & parameters);

  virtual Real computeObjective() override;

protected:
  /// Helper for getting or declaring data
  const std::vector<Real> & getDataValueHelper(const std::string & get_param,
                                               const std::string & declare_param);

  /// vector of simulation data
  const std::vector<Real> & _data_computed;
  /// vector of measured data
  const std::vector<Real> & _data_target;
};
