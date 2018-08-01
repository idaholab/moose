#ifndef GEOMETRICALFLOWCOMPONENT_H
#define GEOMETRICALFLOWCOMPONENT_H

#include "GeometricalComponent.h"
#include "FlowConnection.h"

class GeometricalFlowComponent;

template <>
InputParameters validParams<GeometricalFlowComponent>();

/**
 * Base class for geometrical components that have fluid flow
 */
class GeometricalFlowComponent : public GeometricalComponent
{
public:
  GeometricalFlowComponent(const InputParameters & parameters);

  /// Structure for storing connection data
  struct Connection
  {
    /// Physical position of the connecting point
    Point _position;
    /// Boundary node of connection (used by other components for connecting)
    Node * _node;
    /// Boundary id of this connection
    unsigned int _boundary_id;
    /// Outward norm (either 1 or -1) on boundaries
    Real _normal;

    Connection(const Point & pt, Node * node, unsigned int bc_id, Real normal)
      : _position(pt), _node(node), _boundary_id(bc_id), _normal(normal)
    {
    }
  };

  /**
   * Gets the vector of connections of an end type for this component
   *
   * @param[in] end_type   end type for the connections to get
   */
  virtual const std::vector<Connection> & getConnections(FlowConnection::EEndType end_type) const;

  /**
   * Gets the name of the fluid properties user object for this component
   */
  const UserObjectName & getFluidPropertiesName() const { return _fp_name; }

  /**
   * Gets the flow model ID
   */
  const RELAP7::FlowModelID & getFlowModelID() const;

  /**
   * Gets the RDG conservative flux user object name
   */
  const UserObjectName & getRDGConservativeFluxUserObjectName() const
  {
    return _rdg_conservative_flux_name;
  }

  /**
   * Gets the RDG non-conservative flux user object name
   */
  const UserObjectName & getRDGNonConservativeFluxUserObjectName() const
  {
    return _rdg_nonconservative_flux_name;
  }

  /**
   * Gets the RDG interfacial variables user object name
   */
  const UserObjectName & getRDGInterfacialVariablesUserObjectName() const
  {
    return _rdg_int_var_uo_name;
  }

  /**
   * Gets the flag indicating whether RDG objects are in an implicit form
   */
  const bool & getImplicitRDGFlag() const { return _implicit_rdg; }

protected:
  virtual void init() override;

  /// Map of end type to a list of connections
  std::map<FlowConnection::EEndType, std::vector<Connection>> _connections;

  /// Name of fluid properties user object
  const UserObjectName & _fp_name;

  /// rDG conservative flux user object name
  const UserObjectName _rdg_conservative_flux_name;

  /// rDG non-conservative flux user object name
  const UserObjectName _rdg_nonconservative_flux_name;

  /// rDG interfacial variables user object name
  const UserObjectName _rdg_int_var_uo_name;

  /// Slope reconstruction type for rDG
  const FlowModel::ESlopeReconstructionType _rdg_slope_reconstruction;

  /// Use implicit time integration for rDG
  const bool _implicit_rdg;

  /// Flow model type
  RELAP7::FlowModelID _model_id;
};

#endif /* GEOMETRICALFLOWCOMPONENT_H */
