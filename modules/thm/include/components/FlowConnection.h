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

protected:
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

private:
  /// Vector of connections of this component
  std::vector<Connection> _connections;

  /// Vector of connected component names
  std::vector<std::string> _connected_component_names;
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

namespace RELAP7
{
template <>
FlowConnection::EEndType stringToEnum<FlowConnection::EEndType>(const std::string & s);
}

#endif /* FLOWCONNECTION_H */
