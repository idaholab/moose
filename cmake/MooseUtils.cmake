# Small helpers shared across the MOOSE CMake build.

# moose_collect_subdirs(<out_var> <root>): all directories at/under <root> (incl. <root>).
# MOOSE headers are included by bare name (#include "Foo.h"), so every subdirectory of an
# include tree must be on the include path -- this reproduces the make build's
# `find <dir> -type d`.
function(moose_collect_subdirs out_var root)
  set(_dirs "")
  if(IS_DIRECTORY "${root}")
    list(APPEND _dirs "${root}")
    file(GLOB_RECURSE _entries LIST_DIRECTORIES true "${root}/*")
    foreach(_e IN LISTS _entries)
      if(IS_DIRECTORY "${_e}")
        list(APPEND _dirs "${_e}")
      endif()
    endforeach()
  endif()
  set(${out_var} "${_dirs}" PARENT_SCOPE)
endfunction()
