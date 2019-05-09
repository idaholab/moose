#pragma once

#include "GeometricalComponent.h"

class HeatStructure;

template <>
InputParameters validParams<HeatStructure>();

class HeatStructure : public GeometricalComponent
{
public:
  HeatStructure(const InputParameters & params);

  /// Heat structure geometry type
  enum EHeatStructureType
  {
    PLATE,   ///< Plate geometry
    CYLINDER ///< Cylinder geometry
  };
  /// Map of heat structure geometry type string to enum
  static const std::map<std::string, EHeatStructureType> _hs_type_to_enum;

  /**
   * Gets a MooseEnum for heat structure geometry type
   *
   * @param[in] name   default value
   * @returns MooseEnum for heat structure geometry type
   */
  static MooseEnum getHeatStructureType(const std::string & name = "PLATE");

  virtual void buildMesh() override;

protected:
  virtual void init() override;
  virtual bool usingSecondOrderMesh() const override;
};

namespace THM
{
template <>
HeatStructure::EHeatStructureType stringToEnum(const std::string & s);
}
