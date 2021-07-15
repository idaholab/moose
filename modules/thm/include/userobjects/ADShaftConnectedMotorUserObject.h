#pragma once

#include "GeneralUserObject.h"
#include "ADShaftConnectableUserObjectInterface.h"

/**
 * User object to provide data for a shaft-connected motor
 */
class ADShaftConnectedMotorUserObject : public GeneralUserObject,
                                        public ADShaftConnectableUserObjectInterface
{
public:
  ADShaftConnectedMotorUserObject(const InputParameters & params);

  virtual ADReal getTorque() const override;
  virtual ADReal getMomentOfInertia() const override;
  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

protected:
  /// User defined torque
  const Real & _torque;
  /// User defined moment of intertia
  const Real & _inertia;

public:
  static InputParameters validParams();
};
