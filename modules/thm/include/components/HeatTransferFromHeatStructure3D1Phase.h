#pragma once

#include "HeatTransferFromTemperature1Phase.h"
#include "FlowChannel3DAlignment.h"
#include "MooseEnum.h"

/**
 * Connects a 1-phase flow channel and a 3D heat structure
 */
class HeatTransferFromHeatStructure3D1Phase : public HeatTransferFromTemperature1Phase
{
public:
  HeatTransferFromHeatStructure3D1Phase(const InputParameters & parameters);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  virtual const FEType & getFEType() override;

  virtual void setupMesh() override;
  virtual void init() override;
  virtual void check() const override;

  /// Boundary names for which the boundary component applies
  const BoundaryName & _boundary;

  /// Heat structure name
  const std::string & _hs_name;
  /// Flow channel alignment object
  FlowChannel3DAlignment _fch_alignment;
  /// Direction for layered average user objects
  MooseEnum _layered_average_uo_direction;

public:
  static InputParameters validParams();
};
