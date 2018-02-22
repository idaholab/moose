#ifndef GEOMETRICALCOMPONENT_H
#define GEOMETRICALCOMPONENT_H

#include "Component.h"
#include "RELAP7App.h"
#include "PipeConnectable.h"
#include <map>
#include <unordered_set>
#include <numeric>

class GeometricalComponent;

template <>
InputParameters validParams<GeometricalComponent>();

/**
 * Intermediate class for all geometrical components (i.e components that have position, direction,
 * etc. in space - they generate a mesh)
 */
class GeometricalComponent : public Component
{
public:
  GeometricalComponent(const InputParameters & parameters);

  struct Connection
  {
    /// Physical position of the connecting point
    Point _position;
    /// Boundary node of connection (used by other components for connecting)
    Node * _node;
    /// Boundary id of this connection
    unsigned int _boundary_id;
    /// Out norm (either 1 or -1) on boundaries
    Real _normal;

    Connection(const Point & pt, Node * node, unsigned int bc_id, Real normal)
      : _position(pt), _node(node), _boundary_id(bc_id), _normal(normal)
    {
    }
  };

  /**
   * Method that moves the nodes from reference space into the physical space.
   * Child classes are required to call this in addMooseObjects() if they want to perform such a
   * transformation. Non-straight components can override this method to do their component-specific
   * transformation
   */
  virtual void displaceMesh(const std::vector<SubdomainName> & blocks);

  virtual Point getPosition() const { return _position; }
  virtual RealVectorValue getDirection() const { return _dir; }
  virtual Real getRotation() const { return _rotation; }

  virtual Real getNumElems() const { return _n_elem; }
  virtual Real getLength() const { return _length; }

  virtual const std::vector<Connection> & getConnections(PipeConnectable::EEndType id) const;

  /**
   * Gets the subdomain IDs for this component
   *
   * @return vector of subdomain IDs for this component
   */
  virtual const std::vector<unsigned int> & getSubdomainIds() const;

  /**
   * Gets the subdomain names for this component
   *
   * @return vector of subdomain names for this component
   */
  virtual const std::vector<SubdomainName> & getSubdomainNames() const;

  /**
   * Gets the coordinate system types for this component
   *
   * @return vector of coordinate system types for this component
   */
  virtual const std::vector<Moose::CoordinateSystemType> & getCoordSysTypes() const;

  /**
   * Gets the FE type for this component
   *
   * @return The finite element type
   */
  const FEType & feType() const { return _fe_type; }

protected:
  virtual void check() override;
  virtual void setupMesh() override;

  virtual void buildMesh() = 0;

  /**
   * Sets the next subdomain ID, name, and coordinate system
   *
   * @param[in] subdomain_id  subdomain index
   * @param[in] subdomain_name  name of the new subdomain
   * @param[in] coord_system  type of coordinate system
   */
  virtual void
  setSubdomainInfo(unsigned int subdomain_id,
                   const std::string & subdomain_name,
                   const Moose::CoordinateSystemType & coord_system = Moose::COORD_XYZ);

  const FunctionName & getVariableFn(const FunctionName & fn_param_name);

  /// Physical position in the space
  const Point & _position;

  /// Offset for mesh generation
  const RealVectorValue & _offset;

  /// Direction this pipe is going to
  const RealVectorValue & _dir;

  /// Rotation of the component around x-axis in non-displaced space
  const Real & _rotation;

  /// Length of each subsection of the geometric component
  std::vector<Real> _lengths;

  /// Total length of the geometric component along the main axis
  Real _length;

  /// Number of elements in each subsection of the geometric component
  const std::vector<unsigned int> & _n_elems;

  /// Number of elements along the main axis
  const unsigned int _n_elem;

  /// True if simulation is using a second order mesh
  const bool _2nd_order_mesh;

  /// Number of nodes along the main axis
  const unsigned int _n_nodes;

  /// Number of sections in the geometric component
  const unsigned int _n_sections;

  /// The FE type used in this component
  const FEType _fe_type;

  /// The name of the user object to displace nodes into the physical space
  UserObjectName _displace_node_user_object_name;

  /// Node locations along the main axis
  std::vector<Real> _node_locations;

  std::map<PipeConnectable::EEndType, std::vector<Connection>> _connections;

  /// List of subdomain IDs this components owns
  std::vector<unsigned int> _subdomain_ids;
  /// List of subdomain names this components owns
  std::vector<SubdomainName> _subdomain_names;
  /// List of coordinate system for each subdomain
  std::vector<Moose::CoordinateSystemType> _coord_sys;

private:
  void generateNodeLocations();
  unsigned int computeNumberOfNodes(unsigned int n_elems);
  std::vector<Real> getUniformNodeLocations(Real length, unsigned int n_nodes);
  void placeLocalNodeLocations(Real start_length,
                               unsigned int start_node,
                               std::vector<Real> & local_node_locations);
};

#endif /* GEOMETRICALCOMPONENT_H */
