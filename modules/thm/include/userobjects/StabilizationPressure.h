#pragma once

#include "StabilizationSettings.h"

class StabilizationPressure;
class FlowModelSinglePhase;
class FlowModelTwoPhase;

template <>
InputParameters validParams<StabilizationPressure>();

class StabilizationPressure : public StabilizationSettings
{
public:
  enum EPressureSmootherMethodType
  {
    FIRST_ORDER,
    SECOND_ORDER
  };

  StabilizationPressure(const InputParameters & parameters);

  virtual void addVariables(FlowModel & fm, unsigned int subdomain_id) const;
  virtual void initMooseObjects(FlowModel & fm);
  virtual void addMooseObjects(FlowModel & fm, InputParameters & pars) const;

protected:
  void setup1Phase(FlowModelSinglePhase & fm, InputParameters & pars) const;
  void setup2Phase(FlowModelTwoPhase & fm, InputParameters & pars) const;

  void initMethodTypeMap();
  EPressureSmootherMethodType stringToEnum(const std::string & s);

  const Real & _ce;
  const Real & _ce_liquid;
  const Real & _ce_vapor;

  const Real _scaling_factor_laplacep;
  const Real _scaling_factor_laplacep_liquid;
  const Real _scaling_factor_laplacep_vapor;

  std::map<std::string, EPressureSmootherMethodType> _method_type_to_enum;
  MooseEnum _method_moose_enum;
  EPressureSmootherMethodType _method;

  const bool _use_reference_pressure;
  const Real _p_reference;

  const bool _use_low_mach_fix;

  static const std::string PRESSURE_BAR;
  static const std::string PRESSURE_BAR_LIQUID;
  static const std::string PRESSURE_BAR_VAPOR;
  static const std::string LAPLACE_P;
  static const std::string LAPLACE_P_LIQUID;
  static const std::string LAPLACE_P_VAPOR;
};
