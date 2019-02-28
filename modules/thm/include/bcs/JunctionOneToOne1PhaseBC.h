#ifndef JUNCTIONONETOONE1PHASEBC_H
#define JUNCTIONONETOONE1PHASEBC_H

#include "OneDIntegratedBC.h"

class JunctionOneToOne1PhaseBC;
class JunctionOneToOne1PhaseUserObject;

template <>
InputParameters validParams<JunctionOneToOne1PhaseBC>();

/**
 * Adds boundary fluxes for flow channels connected to a 1-phase one-to-one junction
 */
class JunctionOneToOne1PhaseBC : public OneDIntegratedBC
{
public:
  JunctionOneToOne1PhaseBC(const InputParameters & params);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  virtual void computeJacobian() override;
  virtual void computeJacobianBlock(MooseVariableFEBase & jvar) override;
  virtual void computeJacobianBlock(unsigned jvar) override;

  /**
   * Creates mapping of coupled variable index to local equation system index
   */
  virtual std::map<unsigned int, unsigned int> getIndexMapping() const;

  /// Index of connected flow channel
  const unsigned int _connection_index;

  /// 1-phase one-to-one junction user object
  const JunctionOneToOne1PhaseUserObject & _junction_uo;

  /// Cross-sectional area, elemental
  const VariableValue & _A_elem;
  /// Cross-sectional area, linear
  const VariableValue & _A_linear;

  /// rho*A coupled variable index
  const unsigned int _rhoA_jvar;
  /// rho*u*A coupled variable index
  const unsigned int _rhouA_jvar;
  /// rho*E*A coupled variable index
  const unsigned int _rhoEA_jvar;

  /// Map of coupled variable index to local equation system index
  const std::map<unsigned int, unsigned int> _jvar_map;
  /// Index within local system of the equation upon which this object acts
  const unsigned int _equation_index;
};

#endif
