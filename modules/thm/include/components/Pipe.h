#ifndef PIPE_H
#define PIPE_H

#include "PipeBase.h"

class Pipe;
class ClosuresBase;

template <>
InputParameters validParams<Pipe>();

/**
 * A simple pipe component
 *
 * A pipe is defined by its position, direction, length and area.
 * Mesh: mesh is generated in such a way, that the pipe starts at the origin (0, 0, 0) and is
 * aligned with x-axis. It's subdivided into _n_elems elements (of type EDGE2).
 */
class Pipe : public PipeBase
{
public:
  Pipe(const InputParameters & params);

  virtual void buildMesh() override;
  virtual void addVariables() override;
  virtual void addMooseObjects() override;

  const std::vector<unsigned int> & getNodeIDs() const;
  const std::vector<unsigned int> & getElementIDs() const;
  unsigned int getNodesetID() const;
  const BoundaryName & getNodesetName() const;
  unsigned int getSubdomainID() const override;
  virtual bool isHorizontal() const override { return _is_horizontal; }

  /**
   * Gets heat transfer geometry
   */
  EConvHeatTransGeom getHeatTransferGeometry() const { return _HT_geometry; }

  /**
   * Gets temperature mode flag
   */
  bool getTemperatureMode() const { return _temperature_mode; }

  /**
   * Gets the number of heat transfer connections
   */
  unsigned int getNumberOfHeatTransferConnections() const { return _n_heat_transfer_connections; }

  /**
   * Gets 1-phase wall heat transfer coefficient names for connected heat transfers
   */
  std::vector<MaterialPropertyName> getWallHTCNames1Phase() const { return _Hw_1phase_names; }

  /**
   * Gets liquid wall heat transfer coefficient names for connected heat transfers
   */
  std::vector<MaterialPropertyName> getWallHTCNamesLiquid() const { return _Hw_liquid_names; }

  /**
   * Gets vapor wall heat transfer coefficient names for connected heat transfers
   */
  std::vector<MaterialPropertyName> getWallHTCNamesVapor() const { return _Hw_vapor_names; }

  /**
   * Gets heated perimeter names for connected heat transfers
   */
  std::vector<VariableName> getHeatedPerimeterNames() const { return _P_hf_names; }

  /**
   * Gets wall temperature names for connected heat transfers
   */
  std::vector<VariableName> getWallTemperatureNames() const { return _T_wall_names; }

  /**
   * Adds the name of a heat transfer component to the pipe's list.
   *
   * This function is called from a heat transfer component to add its name to
   * the pipe's list, so that the pipe can communicate with it, such as for
   * determining variable names used in averages of heat transfer variables.
   *
   * @param[in] ht_name  name of the heat transfer component
   */
  void addHeatTransferName(const std::string & ht_name) const;

  /**
   * Gets suffix to add to heat-transfer-related names in a heat transfer component
   *
   * This function is called from a heat transfer component to determine how to
   * name its variables. If the pipe has only one heat transfer coefficient, for
   * example, then no suffix is returned. This function must be called after all
   * of the heat transfer component names have been added to the pipe's list
   * (via \c addHeatTransferName()).
   *
   * @param[in] ht_name  name of the heat transfer component
   * @return suffix to add to heat transfer variable names
   */
  std::string getHeatTransferNamesSuffix(const std::string & ht_name) const;

protected:
  virtual void init() override;
  virtual void initSecondary() override;
  virtual void check() const override;

  virtual std::shared_ptr<ClosuresBase> buildClosures();

  virtual void buildMeshNodes();

  /**
   * Adds objects which are common for single- and two-phase flow
   */
  virtual void addCommonObjects();

  /// The name of used closures
  const MooseEnum & _closures_name;

  /// Closures object
  std::shared_ptr<ClosuresBase> _closures;

  /// True if the user provided a function describing the area of the pipe
  bool _const_A;

  const bool & _pipe_pars_transferred;

  /// True if the user provided a function describing the hydraulic diameter of the pipe
  bool _has_Dh;
  /// Function describing the hydraulic diameter
  FunctionName _Dh_function;

  /// Roughness of pipe surface, [m]
  const Real & _roughness;

  /// Convective Heat transfer geometry
  EConvHeatTransGeom _HT_geometry;
  /// Pitch to diameter ratio for parallel bundle heat transfer
  const Real & _PoD;
  /// True if user provides PoD
  bool _has_PoD;

  /// Subdomain id this pipe defined
  unsigned int _subdomain_id;
  /// Nodeset id for all pipe nodes
  unsigned int _nodeset_id;
  /// Nodeset name for all pipe nodes
  BoundaryName _nodeset_name;

  /// Nodes ids of this pipe component
  std::vector<unsigned int> _node_ids;
  /// Elements ids of this pipe component
  std::vector<unsigned int> _elem_ids;

  /// True if user provides a constant interface f
  bool _const_f_interface;
  /// Interface friction
  const Real & _f_interface;

  /// Flag that pipe is "horizontal"; else "vertical". The pipe is considered
  /// "horizontal" if the angle between the pipe orientation vector and gravity
  /// vector is between 45 degrees and 135 degrees.
  const bool _is_horizontal;

  /// The name of the user object that will set up stabilization
  UserObjectName _stabilization_uo_name;

  /// True if there is one or more sources specified by wall temperature
  bool _temperature_mode;
  /// Names of the heat transfer components connected to this component
  mutable std::vector<std::string> _heat_transfer_names;
  /// Number of connected heat transfer components
  mutable unsigned int _n_heat_transfer_connections;

  virtual void setup1Phase();
  virtual void setup2Phase();

  virtual void setupVolumeFraction();
  virtual void setupDh();
  virtual void addFormLossObjects();
  virtual void addWallTemperatureObjects();

  /**
   * Populates heat connection variable names lists
   */
  virtual void getHeatTransferVariableNames();

  /// 1-phase wall heat transfer coefficient names for connected heat transfers
  std::vector<MaterialPropertyName> _Hw_1phase_names;
  /// liquid wall heat transfer coefficient names for connected heat transfers
  std::vector<MaterialPropertyName> _Hw_liquid_names;
  /// vapor wall heat transfer coefficient names for connected heat transfers
  std::vector<MaterialPropertyName> _Hw_vapor_names;
  /// heated perimeter names for connected heat transfers
  std::vector<VariableName> _P_hf_names;
  /// wall temperature names for connected heat transfers
  std::vector<VariableName> _T_wall_names;
  /// wall heat flux names for connected heat transfers
  std::vector<VariableName> _q_wall_names;
};

#endif /* PIPE_H */
