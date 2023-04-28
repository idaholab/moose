#pragma once

#include "ElementUserObject.h"
#include "RadialAverage.h"

class DensitySensitivities : public ElementUserObject
{
public:
  static InputParameters validParams();

  DensitySensitivities(const InputParameters & parameters);

  virtual void initialize() override{};
  virtual void execute() override;
  virtual void finalize() override{};
  virtual void threadJoin(const UserObject &) override{};

protected:
  const RadialAverage::Result & _filter;
  MooseVariable & _density_sensitivity;
  const VariableName _design_density_name;
  MooseVariable & _design_density;
};
