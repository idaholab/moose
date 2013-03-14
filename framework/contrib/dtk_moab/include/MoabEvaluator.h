//---------------------------------------------------------------------------//
/*!
 * \file MoabEvaluator.hpp
 * \author Stuart R. Slattery
 * \brief Moab function evaluator declaration.
 */
//---------------------------------------------------------------------------//

#ifndef DTK_MOABEVALUATOR_EX_HPP
#define DTK_MOABEVALUATOR_EX_HPP

#include "libmesh/libmesh_config.h"

#ifdef LIBMESH_HAVE_DTK

#include "MoabMesh.h"
#include "ArrayField.h"

#include <DTK_FieldEvaluator.hpp>

class MoabEvaluator
    : public DataTransferKit::FieldEvaluator<MoabMesh::global_ordinal_type,
					     ArrayField>
{
public:

  typedef MoabMesh::global_ordinal_type global_ordinal_type;

  MoabEvaluator( const MoabMesh& mesh, std::string in_var_name )
      : d_mesh( mesh ),
        var_name(in_var_name)
    {}

  ~MoabEvaluator()
    {}

  ArrayField evaluate( const Teuchos::ArrayRCP<global_ordinal_type>& elements,
                       const Teuchos::ArrayRCP<double>& coords );

private:
  MoabMesh d_mesh;

  std::string var_name;
};

#endif // end DTK_MOABEVALUATOR_EX_HPP

//---------------------------------------------------------------------------//
// end MoabEvaluator.hpp
//---------------------------------------------------------------------------//

#endif // LIBMESH_HAVE_DTK
