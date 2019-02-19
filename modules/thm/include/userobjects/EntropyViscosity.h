#ifndef ENTROPYVISCOSITY_H
#define ENTROPYVISCOSITY_H

#include "StabilizationSettings.h"

class EntropyViscosity;
class FlowModelSinglePhase;
class FlowModelTwoPhase;

template <>
InputParameters validParams<EntropyViscosity>();

class EntropyViscosity : public StabilizationSettings
{
public:
  EntropyViscosity(const InputParameters & parameters);

  virtual void addVariables(FlowModel & fm, unsigned int subdomain_id) const;
  virtual void initMooseObjects(FlowModel & fm);
  virtual void addMooseObjects(FlowModel & fm, InputParameters & pars) const;

protected:
  void setup1Phase(FlowModelSinglePhase & fm, InputParameters & pars) const;
  void setup2Phase(FlowModelTwoPhase & fm, InputParameters & pars) const;

  // single phase
  const Real & _Cmax;
  const Real & _Cjump;
  const Real & _Centropy;
  const bool & _use_first_order;
  const bool & _use_parabolic_regularization;
  const bool & _use_jump;
  const MooseEnum & _mach_regime_function;
  const Real & _M_thres;
  const Real & _a;
  // two phase
  const Real & _Cmax_vf;
  const Real & _Cjump_vf;
  const Real & _Centropy_vf;
  const bool & _use_first_order_vf;
  const bool & _use_jump_vf;
  // Liquid
  const Real & _Cmax_liquid;
  const Real & _Cjump_liquid;
  const Real & _Centropy_liquid;
  const bool & _use_first_order_liquid;
  const bool & _use_parabolic_regularization_liquid;
  const bool & _use_jump_liquid;
  const MooseEnum & _mach_regime_function_liquid;
  const Real & _M_thres_liquid;
  const Real & _a_liquid;
  // Vapor
  const Real & _Cmax_vapor;
  const Real & _Cjump_vapor;
  const Real & _Centropy_vapor;
  const bool & _use_first_order_vapor;
  const bool & _use_parabolic_regularization_vapor;
  const bool & _use_jump_vapor;
  const MooseEnum & _mach_regime_function_vapor;
  const Real & _M_thres_vapor;
  const Real & _a_vapor;

  const bool _use_low_mach_fix;

protected:
  static const std::string MAX_VISCOSITY;
  static const std::string KAPPA;
  static const std::string MU;
  static const std::string RESIDUAL;

  static const std::string MAX_VISCOSITY_LIQUID;
  static const std::string MAX_VISCOSITY_VAPOR;
  static const std::string KAPPA_LIQUID;
  static const std::string KAPPA_VAPOR;
  static const std::string MU_LIQUID;
  static const std::string MU_VAPOR;
  static const std::string BETA_LIQUID;
  static const std::string BETA_VAPOR;
  static const std::string MAX_BETA_LIQUID;
  static const std::string MAX_BETA_VAPOR;

  static const std::string JUMP_PRESSURE;
  static const std::string JUMP_RHO;

  static const std::string JUMP_PRESSURE_LIQUID;
  static const std::string JUMP_RHO_LIQUID;
  static const std::string JUMP_PRESSURE_VAPOR;
  static const std::string JUMP_RHO_VAPOR;
  static const std::string JUMP_ALPHA;
};

#endif /* ENTROPYVISCOSITY_H */
