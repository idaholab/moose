/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#pragma once

#include "GeneralUserObject.h"
#include "ExtraQPProvider.h"

/**
 * Manage the history of stateful extra QP material properties. This is sooper-dee-dooper
 * experimental!
 */
class XFEMMaterialManager : public GeneralUserObject, public ExtraQPProvider
{
public:
  static InputParameters validParams();

  XFEMMaterialManager(const InputParameters & parameters);
  ~XFEMMaterialManager();

  virtual void timestepSetup() override;
  virtual void initialSetup() override;

  virtual void rewind();

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

  /// API call to swap in properties
  void swapInProperties(dof_id_type elem_id);
  void swapOutProperties(dof_id_type elem_id);
  void swapInProperties(dof_id_type elem_id) const;
  void swapOutProperties(dof_id_type elem_id) const;

  virtual const std::map<dof_id_type, std::vector<Point>> & getExtraQPMap() const override
  {
    return _extra_qp_map;
  };

  ///@{ API calls to fetch a materialProperty
  template <typename T>
  const MaterialProperty<T> & getMaterialProperty(const std::string & name) const;
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOld(const std::string & name) const;
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOlder(const std::string & name) const;
  ///@}

protected:
  unsigned int materialPropertyIndex(const std::string & name) const;

  /// underlying libMesh mesh
  const MeshBase & _mesh;

  ///@{ Convenience links to the MaterialData object. This is what materials see.
  MaterialProperties _props;
  MaterialProperties _props_old;
  MaterialProperties _props_older;
  //@}

  ///@{ Materials managed by this object and their properties
  std::vector<MaterialBase *> _materials;
  MaterialProperties _properties;
  ///@}

  using HistoryStorage = std::map<dof_id_type, MaterialProperties>;

  ///@{ storage for properties on all elements
  std::unique_ptr<HistoryStorage> _map;
  std::unique_ptr<HistoryStorage> _map_old;
  std::unique_ptr<HistoryStorage> _map_older;
  ///@}

  /// Extra QPs
  const std::map<dof_id_type, std::vector<Point>> & _extra_qp_map;

  /// map from property names to indes into _props etc.
  std::map<std::string, unsigned int> _managed_properties;
};

template <typename T>
const MaterialProperty<T> &
XFEMMaterialManager::getMaterialProperty(const std::string & name) const
{
  auto prop = dynamic_cast<MaterialProperty<T> *>(_props[materialPropertyIndex(name)]);
  if (prop == nullptr)
    mooseError("Property '", name, "' was requested using the wrong type");

  return *prop;
}

template <typename T>
const MaterialProperty<T> &
XFEMMaterialManager::getMaterialPropertyOld(const std::string & name) const
{
  auto prop = dynamic_cast<MaterialProperty<T> *>(_props_old[materialPropertyIndex(name)]);
  if (prop == nullptr)
    mooseError("Property '", name, "' was requested using the wrong type");

  return *prop;
}

template <typename T>
const MaterialProperty<T> &
XFEMMaterialManager::getMaterialPropertyOlder(const std::string & name) const
{
  auto prop = dynamic_cast<MaterialProperty<T> *>(_props_older[materialPropertyIndex(name)]);
  if (prop == nullptr)
    mooseError("Property '", name, "' was requested using the wrong type");

  return *prop;
}
