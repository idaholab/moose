#pragma once

#include "Component.h"

class FormLossFromFunction1Phase;

template <>
InputParameters validParams<FormLossFromFunction1Phase>();

/**
 * Component for prescribing a form loss over a 1-phase flow channel given by a function
 */
class FormLossFromFunction1Phase : public Component
{
public:
  FormLossFromFunction1Phase(const InputParameters & params);

  virtual void check() const override;
  virtual void addMooseObjects() override;

protected:
  /// Name of the flow channel component
  const std::string & _flow_channel_name;
};
