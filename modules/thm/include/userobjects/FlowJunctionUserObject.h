#ifndef FLOWJUNCTIONUSEROBJECT_H
#define FLOWJUNCTIONUSEROBJECT_H

#include "SideUserObject.h"

class FlowJunctionUserObject;

template <>
InputParameters validParams<FlowJunctionUserObject>();

/**
 * Provides common interfaces for flow junction user objects
 */
class FlowJunctionUserObject : public SideUserObject
{
public:
  FlowJunctionUserObject(const InputParameters & parameters);

  virtual void finalize() override;

  /**
   * Gets the number of connected flow channels.
   */
  unsigned int getNumberOfConnections() const { return _n_connections; }

protected:
  /**
   * Gets the index of the currently executing boundary within the vector of
   * boundary IDs given to this SideUserObject
   */
  unsigned int getBoundaryIDIndex();

  /**
   * Checks that a connection index is valid.
   *
   * @param[in] connection_index   Connection index
   */
  void checkValidConnectionIndex(const unsigned int & connection_index) const;

  /**
   * Vector of boundary IDs for this side user object; note that BoundaryRestrictable
   * stores these same boundary IDs in _bnd_ids, but they are stored privately
   * and in a set instead of a vector, so ordering is not preserved.
   */
  const std::vector<BoundaryID> _bnd_ids_vector;
  const unsigned int _n_bnd_ids;
  std::map<std::pair<const Elem *, const unsigned short int>, unsigned int>
      _elem_side_to_bnd_id_index;

  /// Flow channel outward normals or junction inward normals
  const std::vector<Real> & _normal;
  /// Direction of the element connected to the junction
  const MaterialProperty<RealVectorValue> & _dir;
  /// Number of connected flow channels
  const unsigned int _n_connections;
};

#endif /* FLOWJUNCTIONUSEROBJECT_H */
