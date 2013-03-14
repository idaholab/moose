//---------------------------------------------------------------------------//
/*!
 * \file MoabMesh.hpp
 * \author Stuart R. Slattery
 * \brief Moab mesh declaration for example.
 */
//---------------------------------------------------------------------------//

#ifndef DTK_MOABMESH_EX_HPP
#define DTK_MOABMESH_EX_HPP

#include <string>

#include "ArrayField.h"

#include <DTK_MeshContainer.hpp>
#include <DTK_MeshTypes.hpp>

#include <MBInterface.hpp>
#include <MBRange.hpp>

#include <Teuchos_RCP.hpp>
#include <Teuchos_Comm.hpp>
#include <Teuchos_ArrayRCP.hpp>

//---------------------------------------------------------------------------//
class MoabMesh
{
  public:

    typedef moab::EntityHandle                        global_ordinal_type;
    typedef Teuchos::Comm<int>                        CommType;
    typedef Teuchos::RCP<const CommType>              RCP_Comm;
    typedef DataTransferKit::MeshContainer<moab::EntityHandle> Container;

    MoabMesh()
    { /* ... */ }

    MoabMesh( const RCP_Comm& comm,
	      const std::string& filename,
	      const moab::EntityType& block_topology,
	      const int partitioning_type );

    ~MoabMesh()
    { /* ... */ }

    const Teuchos::RCP<moab::Interface>& getMoab() const
    { return d_moab; }

    void tag(std::string var_name, const ArrayField& data);

    void write( const std::string& filename );

    Teuchos::RCP<Container> meshContainer() const
    { return d_mesh_container; }

  private:

    DataTransferKit::DTK_ElementTopology
    getTopology( moab::EntityType topology ) const;

  private:

    // Communicator.
    RCP_Comm d_comm;

    // Moab interface.
    Teuchos::RCP<moab::Interface> d_moab;

    // DTK mesh container.
    Teuchos::RCP<Container> d_mesh_container;

    // Reference counting pointer to mesh vertices (for tagging only).
    Teuchos::ArrayRCP<moab::EntityHandle> d_vertices;
};

//---------------------------------------------------------------------------//

#endif // end DTK_MOABMESH_EX_HPP

//---------------------------------------------------------------------------//
// end MoabMesh.hpp
//---------------------------------------------------------------------------//

