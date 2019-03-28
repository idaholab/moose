#ifndef JUNCTIONSTAGNATIONENTHALPYUSEROBJECT_H
#define JUNCTIONSTAGNATIONENTHALPYUSEROBJECT_H

#include "FlowJunctionUserObject.h"
#include "DerivativeMaterialInterfaceTHM.h"

class JunctionStagnationEnthalpyUserObject;

template <>
InputParameters validParams<JunctionStagnationEnthalpyUserObject>();

/**
 * Computes the stagnation enthalpy associated with a junction, as well as its
 * derivatives associated with degrees of freedom on each of the connected flow channels.
 */
class JunctionStagnationEnthalpyUserObject
  : public DerivativeMaterialInterfaceTHM<FlowJunctionUserObject>
{
public:
  JunctionStagnationEnthalpyUserObject(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & uo) override;

  Real getJunctionStagnationEnthalpy() const;
  const std::vector<Real> & getJunctionStagnationEnthalpyMassDerivatives() const;
  const std::vector<Real> & getJunctionStagnationEnthalpyMomentumDerivatives() const;
  const std::vector<Real> & getJunctionStagnationEnthalpyEnergyDerivatives() const;

protected:
  Real _H_junction;
  std::vector<Real> _dH_junction_drhoA;
  std::vector<Real> _dH_junction_drhouA;
  std::vector<Real> _dH_junction_drhoEA;

  Real _H_sum;
  std::vector<Real> _dH_sum_drhoA;
  std::vector<Real> _dH_sum_drhouA;
  std::vector<Real> _dH_sum_drhoEA;

  Real _energy_rate_in;
  std::vector<Real> _denergy_rate_in_drhoA;
  std::vector<Real> _denergy_rate_in_drhouA;
  std::vector<Real> _denergy_rate_in_drhoEA;

  Real _mass_flow_rate_in;
  std::vector<Real> _dmass_flow_rate_in_drhoA;
  std::vector<Real> _dmass_flow_rate_in_drhouA;

  const VariableValue & _A;
  const VariableValue & _rhouA_old;

  const MaterialProperty<Real> & _rho;
  const MaterialProperty<Real> & _drho_drhoA;

  const MaterialProperty<Real> & _vel;
  const MaterialProperty<Real> & _dvel_drhoA;
  const MaterialProperty<Real> & _dvel_drhouA;

  const MaterialProperty<Real> & _H;
  const MaterialProperty<Real> & _dH_drhoA;
  const MaterialProperty<Real> & _dH_drhouA;
  const MaterialProperty<Real> & _dH_drhoEA;
};

#endif /* JUNCTIONSTAGNATIONENTHALPYUSEROBJECT_H */
