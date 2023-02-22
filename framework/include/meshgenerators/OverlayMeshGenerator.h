//* https://www.gnu.org/licenses/lgpl-2.1.html
#pragma once
#include "MeshGenerator.h"
#include "MooseEnum.h"
/*
 * Mesh generator to create a Overlay mesh
 */
class OverlayMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();
  OverlayMeshGenerator(const InputParameters & parameters);
  std::unique_ptr<MeshBase> generate() override;

protected:
  /// The dimension of the mesh
  MooseEnum _dim;

  /// Name of the generated mesh
  std::unique_ptr<MeshBase> * _build_mesh;

  /// Name of the base mesh
  std::unique_ptr<MeshBase> & _mesh_input;
};
