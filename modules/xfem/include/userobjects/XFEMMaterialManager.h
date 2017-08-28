/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef XFEMMATERIALMANAGER_H
#define XFEMMATERIALMANAGER_H

#include "GeneralUserObject.h"

/**
 * Manage the history of stateful extra QP material properties. This is sooper-dee-dooper
 * experimental!
 */
class XFEMMaterialManager : public GeneralUserObject
{
public:
  XFEMMaterialManager(const InputParameters & parameters);
  ~XFEMMaterialManager();

  virtual void rewind();

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

  /// Public API to instruct the object to swap in properties
  void swapInProperties(dof_id_type elem_id) const;

protected:
  /// underlying libMesh mesh
  const MeshBase & _mesh;

  ///@{ Convenience links to the MaterialData object. This is what materials see.
  MaterialProperties _props;
  MaterialProperties _props_old;
  MaterialProperties _props_older;
  //@}

  ///@{ Materials managed by this object and their properties
  std::vector<Material *> _materials;
  MaterialProperties _properties;
  ///@}

  using HistoryStorage = std::map<dof_id_type, MaterialProperties>;

  ///@{ storage for properties on all elements
  std::unique_ptr<HistoryStorage> _map;
  std::unique_ptr<HistoryStorage> _map_old;
  std::unique_ptr<HistoryStorage> _map_older;
  ///@}

  std::map<dof_id_type, std::vector<Point>> _extra_qp_map;
};

template <>
InputParameters validParams<XFEMMaterialManager>();

#endif // XFEMMATERIALMANAGER_H
