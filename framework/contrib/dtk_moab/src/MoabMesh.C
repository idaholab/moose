//---------------------------------------------------------------------------//
/*!
 * \file MoabMesh.cpp
 * \author Stuart R. Slattery
 * \brief Moab mesh definition more example.
 */
//---------------------------------------------------------------------------//

#include "libmesh/libmesh_config.h"

#ifdef LIBMESH_HAVE_DTK

#include <cassert>
#include <vector>
#include <algorithm>


#include "MoabMesh.h"

#include <MBCore.hpp>

#include <Teuchos_as.hpp>
#include <Teuchos_Array.hpp>

//---------------------------------------------------------------------------//
/*
 * \brief This constructor will pull the mesh data DTK needs out of Moab,
 * partition it for the example, and build a DataTransferKit::MeshContainer
 * object from the local data in the partition. You can directly write the
 * traits interface yourself, but this is probably the easiest way to get
 * started (although potentially inefficient).
 */
MoabMesh::MoabMesh( const RCP_Comm& comm,
		    const std::string& filename,
		    const moab::EntityType& block_topology,
		    const int partitioning_type )
    : d_comm( comm )
{
    // Compute the node dimension.
    int node_dim = 0;
    if ( block_topology == moab::MBTRI )
    {
	node_dim = 2;
    }
    else if ( block_topology == moab::MBQUAD )
    {
	node_dim = 2;
    }
    else if ( block_topology == moab::MBTET )
    {
	node_dim = 3;
    }
    else if ( block_topology == moab::MBHEX )
    {
	node_dim = 3;
    }
    else if ( block_topology == moab::MBPYRAMID )
    {
	node_dim = 3;
    }
    else
    {
	node_dim = 0;
    }

    // Create a moab instance.
    moab::ErrorCode error;
    d_moab = Teuchos::rcp( new moab::Core() );

    std::cout<<"Filename: "<<filename<<std::endl;

    // Load the mesh.
    d_moab->load_mesh( &filename[0] );
    moab::EntityHandle root_set = d_moab->get_root_set();

    // Extract the elements with this block's topology.
    std::vector<moab::EntityHandle> global_elements;
    error = d_moab->get_entities_by_type(
	root_set, block_topology, global_elements );
    assert( error == moab::MB_SUCCESS );

    std::cout<<"Global elements: "<<global_elements.size()<<std::endl;

    // Partition the mesh.
    int comm_size = d_comm->getSize();
    int comm_rank = d_comm->getRank();

    // Get the number of nodes in an element.
    std::vector<moab::EntityHandle> elem_vertices;
    error = d_moab->get_adjacencies( &global_elements[0],
				     1,
				     0,
				     false,
				     elem_vertices );
    assert( error == moab::MB_SUCCESS );
    int nodes_per_element = elem_vertices.size();

    // Get the global element coordinates.
    std::vector<double> global_coords;
    error = d_moab->get_vertex_coordinates( global_coords );
    assert( error == moab::MB_SUCCESS );

    // Get the global max and min values for the coordinates. This problem is
    // symmetric.
    double min = *(std::min_element( global_coords.begin(),
				     global_coords.end() ) );
    double max = *(std::max_element( global_coords.begin(),
				     global_coords.end() ) );
    double width = max - min;

    Teuchos::Array<moab::EntityHandle> elements;
    elem_vertices.resize( nodes_per_element );
    std::vector<double> elem_coords( 3*nodes_per_element );
    std::vector<moab::EntityHandle>::const_iterator global_elem_iterator;
    for ( global_elem_iterator = global_elements.begin();
	  global_elem_iterator != global_elements.end();
	  ++global_elem_iterator )
    {
	// Get the individual element vertices.
	error = d_moab->get_adjacencies( &*global_elem_iterator,
					 1,
					 0,
					 false,
					 elem_vertices );
	assert( error == moab::MB_SUCCESS );

	// Get the invidivual element coordinates.
	error = d_moab->get_coords( &elem_vertices[0],
				    elem_vertices.size(),
				    &elem_coords[0] );
	assert( error == moab::MB_SUCCESS );

	// Partition in x direction.
	if ( partitioning_type == 0 )
	{
	    for ( int i = 0; i < comm_size; ++i )
	    {
		if ( elem_coords[0] >= min + width*(comm_rank)/comm_size - 1e-6 &&
		     elem_coords[0] <= min + width*(comm_rank+1)/comm_size + 1e-6 )
		{
		    elements.push_back( *global_elem_iterator );
		}
	    }
	}

	// Partition in y direction.
	else if ( partitioning_type == 1 )
	{
	    for ( int i = 0; i < comm_size; ++i )
	    {
		if ( elem_coords[1] >= min + width*(comm_rank)/comm_size - 1e-6 &&
		     elem_coords[1] <= min + width*(comm_rank+1)/comm_size + 1e-6 )
		{
		    elements.push_back( *global_elem_iterator );
		}

	    }
	}
	else
	{
	    throw std::logic_error( "Partitioning type not supported." );
	}
    }
    Teuchos::ArrayRCP<moab::EntityHandle> elements_arcp( elements.size() );
    std::copy( elements.begin(), elements.end(), elements_arcp.begin() );
    elements.clear();
    d_comm->barrier();

    // Get the nodes.
    std::vector<moab::EntityHandle> vertices;
    error = d_moab->get_connectivity( &elements_arcp[0],
				      elements_arcp.size(),
				      vertices );
    assert( error == moab::MB_SUCCESS );
    d_vertices = Teuchos::ArrayRCP<moab::EntityHandle>( vertices.size() );
    std::copy( vertices.begin(), vertices.end(), d_vertices.begin() );
    vertices.clear();

    // Get the node coordinates.
    Teuchos::ArrayRCP<double> coords( node_dim * d_vertices.size() );
    std::vector<double> interleaved_coords( 3*d_vertices.size() );
    error = d_moab->get_coords( &d_vertices[0], d_vertices.size(),
				&interleaved_coords[0] );
    assert( error == moab::MB_SUCCESS );

    for ( int n = 0; n < (int) d_vertices.size(); ++n )
    {
	for ( int d = 0; d < (int) node_dim; ++d )
	{
	    coords[ d*d_vertices.size() + n ] =
		interleaved_coords[ n*3 + d ];
	}
    }
    interleaved_coords.clear();

    // Get the connectivity.
    int connectivity_size = elements_arcp.size() * nodes_per_element;
    Teuchos::ArrayRCP<moab::EntityHandle> connectivity( connectivity_size );
    std::vector<moab::EntityHandle> elem_conn;
    for ( int i = 0; i < (int) elements_arcp.size(); ++i )
    {
	error = d_moab->get_connectivity( &elements_arcp[i], 1, elem_conn );

	assert( error == moab::MB_SUCCESS );
	assert( elem_conn.size() ==
		Teuchos::as<std::vector<moab::EntityHandle>::size_type>(nodes_per_element) );

	for ( int n = 0; n < (int) elem_conn.size(); ++n )
	{
	    connectivity[ n*elements_arcp.size() + i ] = elem_conn[n];
	}
    }

    // Get the permutation vector.
    Teuchos::ArrayRCP<int> permutation_list( nodes_per_element );
    for ( int i = 0; i < (int) nodes_per_element; ++i )
    {
	permutation_list[i] = i;
    }

    // Create the mesh container.
    d_mesh_container = Teuchos::rcp(
	new Container( node_dim,
		       d_vertices,
		       coords,
		       getTopology(block_topology),
		       nodes_per_element,
		       elements_arcp,
		       connectivity,
		       permutation_list ) );
}

//---------------------------------------------------------------------------//
void MoabMesh::tag(std::string var_name, const ArrayField& data)
{
    assert( data.size() == d_vertices.size() );

    moab::ErrorCode error;
    moab::Tag tag;

    error = d_moab->tag_get_handle( var_name.c_str(), 1, moab::MB_TYPE_DOUBLE,
				    tag, moab::MB_TAG_CREAT|moab::MB_TAG_DENSE );
    assert( error == moab::MB_SUCCESS );

    error = d_moab->tag_set_data( tag,
				  d_vertices.getRawPtr(),
				  d_vertices.size(),
				  &data[0] );
    assert( error == moab::MB_SUCCESS );
}

//---------------------------------------------------------------------------//
void MoabMesh::write( const std::string& filename )
{
    std::string filename0 = filename + ".0";
    std::string filename1 = filename + ".1";
    std::string filename2 = filename + ".2";
    std::string filename3 = filename + ".3";

    if ( d_comm->getRank() == 0 )
    {
	d_moab->write_mesh( &filename0[0] );
    }
    else if ( d_comm->getRank() == 1 )
    {
	d_moab->write_mesh( &filename1[0] );
    }
    else if ( d_comm->getRank() == 2 )
    {
	d_moab->write_mesh( &filename2[0] );
    }
    else if ( d_comm->getRank() == 3 )
    {
	d_moab->write_mesh( &filename3[0] );
    }
    d_comm->barrier();
}

//---------------------------------------------------------------------------//
DataTransferKit::DTK_ElementTopology
MoabMesh::getTopology( moab::EntityType topology ) const
{
    if ( topology == moab::MBTRI )
    {
	return DataTransferKit::DTK_TRIANGLE;
    }
    else if ( topology == moab::MBQUAD )
    {
	return DataTransferKit::DTK_QUADRILATERAL;
    }
    else if ( topology == moab::MBTET )
    {
	return DataTransferKit::DTK_TETRAHEDRON;
    }
    else if ( topology == moab::MBHEX )
    {
	return DataTransferKit::DTK_HEXAHEDRON;
    }
    else if ( topology == moab::MBPYRAMID )
    {
	return DataTransferKit::DTK_PYRAMID;
    }
    else
    {
	throw std::logic_error( "Topology type not supported." );
    }
    return DataTransferKit::DTK_VERTEX;
}

//---------------------------------------------------------------------------//
// end MoabMesh.cpp
//---------------------------------------------------------------------------//

#endif // LIBMESH_HAVE_DTK
