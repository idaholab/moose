#ifndef FLOWCONNECTION_H
#define FLOWCONNECTION_H

#include "Component.h"
#include "Enums.h"

class FlowConnection;

template <>
InputParameters validParams<FlowConnection>();

/**
 * Base class for components that connect to pipes (junctions and boundaries)
 */
class FlowConnection : public Component
{
public:
  FlowConnection(const InputParameters & params);

  /// End type
  enum EEndType
  {
    IN, ///< inlet
    OUT ///< outlet
  };
  /// Map of end type string to enum
  static const std::map<std::string, EEndType> _end_type_to_enum;

  /// Structure for holding data for a connection
  struct Connection
  {
    /// String for the connection
    const std::string _string;

    /// Name of the geometrical component in the connection
    const std::string _geometrical_component_name;

    /// End type for the connection
    const EEndType _end_type;

    Connection(const std::string & connection_string,
               const std::string & geometrical_component_name,
               const EEndType & end_type)
      : _string(connection_string),
        _geometrical_component_name(geometrical_component_name),
        _end_type(end_type)
    {
    }
  };

  /**
   * Returns the vector of connections of this component
   *
   * @returns vector of connections of this component
   */
  const std::vector<Connection> & getConnections() const { return _connections; }

  /**
   * Returns a list of names of pipes that are connected to this component
   *
   * @returns list of names of pipes that are connected to this component
   */
  const std::vector<std::string> & getConnectedComponentNames() const
  {
    return _connected_component_names;
  }

  /**
   * Gets the vector of connected subdomain IDs
   */
  const std::vector<unsigned int> & getConnectedSubdomainIDs() const
  {
    return _connected_subdomain_ids;
  }

  /**
   * Gets the list of boundary nodes connected to this component
   *
   * @return list of connected boundary nodes
   */
  virtual const std::vector<dof_id_type> & getNodeIDs() const;

  /**
   * Gets the boundary names for this component
   *
   * @return boundary names for this component
   */
  const std::vector<BoundaryName> & getBoundaryNames() const;

  /**
   * Gets the name of fluid properties used in all flow connections
   *
   * @return name of fluid properties used in all flow connections
   */
  const UserObjectName & getFluidPropertiesName() const;

protected:
  virtual void setupMesh() override;
  virtual void init() override;
  virtual void check() override;

  /**
   * Adds a connection for this component
   *
   * Components using this interface must call this function on all of their
   * connection strings.
   *
   * @param[in] connection_string   connection string to parse to create connection
   */
  void addConnection(const std::string & connection_string);

  /**
   * Checks that the number of connections is equal to the supplied value
   *
   * @param[in] n_connections   enforced number of connections
   */
  void checkNumberOfConnections(const unsigned int & n_connections) const;

  /**
   * Checks that the size of a vector parameter equals the number of connections
   *
   * @tparam    T       type of element in the vector parameter
   * @param[in] param   vector parameter name
   */
  template <typename T>
  void checkSizeEqualsNumberOfConnections(const std::string & param) const;

  /**
   * Checks that all connections have the same of a certain type of object
   *
   * @tparam    T             type of object to check
   * @param[in] objects       list of objects corresponding to each connection
   * @param[in] description   description of the obect to check
   */
  template <typename T>
  void checkAllConnectionsHaveSame(const std::vector<T> & objects,
                                   const std::string & description) const;

  /// Physical positions of connected pipes
  std::vector<Point> _positions;
  /// Boundary node IDs from connected pipes
  std::vector<dof_id_type> _nodes;
  /// Boundary IDs of connected pipes
  std::vector<unsigned int> _boundary_ids;
  /// Boundary names of connected pipes
  std::vector<BoundaryName> _boundary_names;
  /// Outward normals associated with connected pipes
  std::vector<Real> _normals;
  /// Connection strings
  std::vector<std::string> _connection_strings;

  /// Convenience variable that stores model type
  RELAP7::FlowModelID _flow_model_id;
  /// The name of the fluid property user object
  UserObjectName _fp_name;

private:
  /**
   * Creates a nodeset/sideset name from the connected component name
   *
   * Generally, for boundaries, this will be the name of this component, but
   * junctions will need to combine this component's name with the connected
   * component's name.
   *
   * @param[in] comp_name   name of the connected component
   * @returns nodeset/sideset name for a particular connection
   */
  virtual std::string createBoundaryName(const std::string & comp_name) const = 0;

  /// Vector of connections of this component
  std::vector<Connection> _connections;

  /// Vector of connected component names
  std::vector<std::string> _connected_component_names;

  /// Vector of subdomain IDs of the connected geometrical flow components
  std::vector<unsigned int> _connected_subdomain_ids;
};

template <typename T>
void
FlowConnection::checkSizeEqualsNumberOfConnections(const std::string & param) const
{
  const auto & value = getParam<std::vector<T>>(param);
  if (value.size() != _connections.size())
    logError("The number of entries in '",
             param,
             "' (",
             value.size(),
             ") must equal the number of connections (",
             _connections.size(),
             ")");
}

template <typename T>
void
FlowConnection::checkAllConnectionsHaveSame(const std::vector<T> & objects,
                                            const std::string & description) const
{
  for (const auto & obj : objects)
    if (obj != objects[0])
      logError("All connections must have the same ", description);
}

namespace RELAP7
{
template <>
FlowConnection::EEndType stringToEnum<FlowConnection::EEndType>(const std::string & s);
}

#endif /* FLOWCONNECTION_H */
