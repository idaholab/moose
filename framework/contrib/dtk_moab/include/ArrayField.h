//---------------------------------------------------------------------------//
/*!
 * \file ArrayField.hpp
 * \author Stuart R. Slattery
 * \brief Simple field implementation for example.
 */
//---------------------------------------------------------------------------//

#ifndef DTK_ARRAYFIELD_EX_HPP
#define DTK_ARRAYFIELD_EX_HPP

#include "libmesh/libmesh_config.h"

#ifdef LIBMESH_HAVE_DTK

#include <DTK_FieldTraits.hpp>

#include <Teuchos_Array.hpp>

class ArrayField
{
  public:

    typedef double value_type;
    typedef Teuchos::Array<double>::size_type size_type;
    typedef Teuchos::Array<double>::iterator iterator;
    typedef Teuchos::Array<double>::const_iterator const_iterator;

    ArrayField( size_type size, std::size_t dim )
	: d_dim( dim )
	, d_data( size )
    { /* ... */ }

    ~ArrayField()
    { /* ... */ }

    std::size_t dim() const
    { return d_dim; }

    size_type size() const
    { return d_data.size(); }

    bool empty() const
    { return d_data.empty(); }

    iterator begin()
    { return d_data.begin(); }

    const_iterator begin() const
    { return d_data.begin(); }

    iterator end()
    { return d_data.end(); }

    const_iterator end() const
    { return d_data.end(); }

    Teuchos::Array<double>& getData()
    { return d_data; }

    const Teuchos::Array<double>& getData() const
    { return d_data; }

    value_type& operator[]( size_type n )
    { return d_data[n]; }

    const value_type& operator[]( size_type n ) const
    { return d_data[n]; }

  private:
    std::size_t d_dim;
    Teuchos::Array<double> d_data;
};

//---------------------------------------------------------------------------//
// Field traits definition.
//---------------------------------------------------------------------------//

namespace DataTransferKit
{
template<>
class FieldTraits<ArrayField>
{
  public:

    typedef ArrayField                    field_type;
    typedef double                        value_type;
    typedef ArrayField::size_type         size_type;
    typedef ArrayField::iterator          iterator;
    typedef ArrayField::const_iterator    const_iterator;

    static inline size_type dim( const ArrayField& field )
    { return field.dim(); }

    static inline size_type size( const ArrayField& field )
    { return field.size(); }

    static inline bool empty( const ArrayField& field )
    { return field.empty(); }

    static inline iterator begin( ArrayField& field )
    { return field.begin(); }

    static inline const_iterator begin( const ArrayField& field )
    { return field.begin(); }

    static inline iterator end( ArrayField& field )
    { return field.end(); }

    static inline const_iterator end( const ArrayField& field )
    { return field.end(); }
};

} // end namespace DataTransferKit

//---------------------------------------------------------------------------//

#endif // DTK_ARRAYFIELD_EX_HPP

//---------------------------------------------------------------------------//
// end ArrayField.hpp
//---------------------------------------------------------------------------//

#endif // LIBMESH_HAVE_DTK
