#ifndef HEATTRANSFERFROMHEATSTRUCTURE_H
#define HEATTRANSFERFROMHEATSTRUCTURE_H

#include "HeatTransferFromTemperature.h"

class HeatTransferFromHeatStructure;

template <>
InputParameters validParams<HeatTransferFromHeatStructure>();

/**
 * Connects a pipe and a heat structure
 */
class HeatTransferFromHeatStructure : public HeatTransferFromTemperature
{
public:
  HeatTransferFromHeatStructure(const InputParameters & parameters);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;

  /**
   * Adds 1-phase objects
   */
  void addMooseObjects1Phase();
  /**
   * Adds 2-phase objects
   */
  void addMooseObjects2Phase();

  /**
   * Gets the "master side" (heat structure) boundary name for this connection
   *
   * @return The boundary name of the master side
   */
  const BoundaryName & getMasterSideName() const;
  /**
   * Gets the "slave side" (pipe) nodeset name for this connection
   *
   * @return The nodeset name for the slave side
   */
  const BoundaryName & getSlaveSideName() const;

  /// heat structure name
  const std::string _hs_name;
  /// "side" of the heat structure
  const MooseEnum _hs_side;
};

#endif /* HEATTRANSFERFROMHEATSTRUCTURE_H */
