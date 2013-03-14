//---------------------------------------------------------------------------//
/*!
 * \file MoabEvaluator.cpp
 * \author Stuart R. Slattery
 * \brief Moab function evaluator definition.
 */
//---------------------------------------------------------------------------//

#include "libmesh/libmesh_config.h"

#ifdef LIBMESH_HAVE_DTK

#include <cassert>
#include <cmath>
#include <vector>

#include "MoabEvaluator.h"

ArrayField MoabEvaluator::evaluate(
    const Teuchos::ArrayRCP<global_ordinal_type>& elements,
    const Teuchos::ArrayRCP<double>& coords )
{
    ArrayField evaluations( elements.size(), 1 );

    // If the element is in the local partition, average the tagged values of
    // the source field for that element.
    moab::ErrorCode error;
    for ( int n = 0; n < elements.size(); ++n )
    {
	// Get the element vertices.
	std::vector<moab::EntityHandle> element_verts;
	error =  d_mesh.getMoab()->get_adjacencies( &elements[n],
						    1,
						    0,
						    false,
						    element_verts );
	assert( error == moab::MB_SUCCESS );

	// Extract the vertex coordinates.
	int num_element_verts = element_verts.size();
	std::vector<double> vert_coords( 3 * num_element_verts );
	error = d_mesh.getMoab()->get_coords( &element_verts[0],
					      element_verts.size(),
					      &vert_coords[0] );
	assert( error == moab::MB_SUCCESS );

	// Get the tag for the Moab function.
	moab::Tag Moab_tag;
	error = d_mesh.getMoab()->tag_get_handle( var_name.c_str(),
						  1,
						  moab::MB_TYPE_DOUBLE,
						  Moab_tag );
	assert( error == moab::MB_SUCCESS );

	// Get the field tag data for the vertices.
	std::vector<double> Moab_data( num_element_verts, 0.0 );
	error = d_mesh.getMoab()->tag_get_data( Moab_tag,
						&element_verts[0],
						element_verts.size(),
						&Moab_data[0] );
	assert( error == moab::MB_SUCCESS );

	// For now, just average all the vertex data for the element
	// evaluation result. We have the element vertex coordinates as
	// provided through the interface so we can use some kind of spatially
	// weighted average using the target coordinates at a later point or
	// even use a higher order basis evaluation.
	evaluations[n] = 0.0;
	for ( int i = 0; i < num_element_verts; ++i )
	{
	    evaluations[n] += Moab_data[i];
	}
	evaluations[n] /= num_element_verts;
    }

    return evaluations;
}

//---------------------------------------------------------------------------//
// end MoabEvaluator.cpp
//---------------------------------------------------------------------------//

#endif // LIBMESH_HAVE_DTK
