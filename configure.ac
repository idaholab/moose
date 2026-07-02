AC_INIT([moose], [0.9.0], [https://github.com/idaholab/moose/discussions], [moose], [https://mooseframework.inl.gov])
AC_PREREQ([2.71])

# Infer the source directory as the path to the ./configure script
srcdir=`dirname $0`
top_srcdir=`dirname $0`

test -n "$top_srcdir" &&
ac_top_srcdir_ls_di=`cd "$top_srcdir" && ls -di .` ||
  as_fn_error $? "top source directory cannot be determined"

ac_pwd=`pwd` && test -n "$ac_pwd" &&
ac_pwd_ls_di=`cd "$ac_pwd" && ls -di .` ||
  as_fn_error $? "working directory cannot be determined"

test "X$ac_top_srcdir_ls_di" = "X$ac_pwd_ls_di" ||
  as_fn_error $? "MOOSE configure must be run from the root directory"

AC_MSG_RESULT(---------------------------------------------)
AC_MSG_RESULT(----------- Configuring MOOSE ---------------)
AC_MSG_RESULT(---------------------------------------------)

AC_CONFIG_MACRO_DIR([m4])

# Create a temporary file with e.g. SPARSE_AD
AC_CONFIG_HEADERS([framework/include/base/MooseConfig.h.tmp:framework/include/base/MooseConfig.h.in])
# Prefixes everything in the temporary file with MOOSE_, e.g. MOOSE_SPARSE_AD
AX_PREFIX_CONFIG_H(framework/include/base/MooseConfig.h,MOOSE,framework/include/base/MooseConfig.h.tmp)

AC_ARG_WITH(derivative-size,
            AS_HELP_STRING([--with-derivative-size=<n>], [Specify the size of the derivative backing array]),
            [derivative_size="$withval"],
            [derivative_size=64])

AC_DEFINE_UNQUOTED(AD_MAX_DOFS_PER_ELEM, [$derivative_size], [The size of the derivative backing array])

AC_MSG_RESULT([configuring with derivative backing array size of $derivative_size])

AC_ARG_WITH(libtorch,
            AS_HELP_STRING([--with-libtorch@<:@=ARG@:>@],[Specify if libtorch support should be enabled @<:@ARG=yes or libtorch directory@:>@]),
            [libtorch_support=yes libtorch_library="$withval"],
            [libtorch_support=no])

AC_MSG_RESULT([configuring with libtorch support: $libtorch_support])

AS_IF([test "$libtorch_support" = yes],
      [
        AC_DEFINE(LIBTORCH_ENABLED, 1, [Whether to use libtorch-related code or not])
        AC_SUBST([libtorch_support],[true])
        AS_IF([test "$libtorch_library" = yes],
              [
                    AS_IF([test "${LIBTORCH_DIR+set}" = set],
                          [libtorch_library="${LIBTORCH_DIR}"],
                          [libtorch_library="$ac_pwd/framework/contrib/pytorch/installed"])
              ])
        AC_DEFINE_UNQUOTED(LIBTORCH_DIR, [$libtorch_library], [The directory where libtorch is installed])
        AC_SUBST([libtorch_library])
        AC_MSG_RESULT([Libtorch library path: $libtorch_library])
      ])

AC_ARG_WITH(mfem,
            AS_HELP_STRING([--with-mfem@<:@=ARG@:>@],[Specify if mfem support should be enabled @<:@ARG=yes or mfem directory@:>@]),
            [mfem_support=yes mfem_library="$withval"],
            [mfem_support=no])

AC_MSG_RESULT([configuring with mfem support: $mfem_support])

AS_IF([test "$mfem_support" = yes],
      [
        AC_DEFINE(MFEM_ENABLED, 1, [Whether to use mfem-related code or not])
        AC_SUBST([mfem_support],[true])
        AS_IF([test "$mfem_library" = yes],
              [
                    AS_IF([test "${MFEM_DIR+set}" = set],
                          [mfem_library="${MFEM_DIR}"],
                          [mfem_library="$ac_pwd/framework/contrib/mfem/installed"])
              ])
        AC_DEFINE_UNQUOTED(MFEM_DIR, [$mfem_library], [The directory where mfem is installed])
        AC_SUBST([mfem_library])
        AC_MSG_RESULT([MFEM library path: $mfem_library])
      ])

AC_ARG_WITH(neml2,
            AS_HELP_STRING([--with-neml2@<:@=ARG@:>@],[Specify if neml2 support should be enabled @<:@ARG=yes or neml2 directory@:>@]),
            [neml2_support=yes neml2_library="$withval"],
            [neml2_support=no])

AC_MSG_RESULT([configuring with neml2 support: $neml2_support])

AS_IF([test "$neml2_support" = yes],
      [
        # Check if libtorch is enabled, since neml2 requires it
        AS_IF([test "$libtorch_support" = no],
              [
                AC_MSG_ERROR([NEML2 depends on libtorch. Please enable libtorch support with --with-libtorch])
              ])
        AC_DEFINE(NEML2_ENABLED, 1, [Whether to use neml2-related code or not])
        AC_SUBST([neml2_support],[true])
        AS_IF([test "$neml2_library" = yes],
              [
                    dnl No path was given to --with-neml2. Prefer the NEML2_DIR environment
                    dnl variable; otherwise locate the neml2 installed in the active Python
                    dnl environment (this is where scripts/update_and_rebuild_neml2.sh installs
                    dnl it -- alongside torch). There is no longer a fixed install location to
                    dnl fall back to, so error out with guidance if neither is available.
                    AS_IF([test "${NEML2_DIR+set}" = set],
                          [neml2_library="${NEML2_DIR}"],
                          dnl Use importlib rather than neml2.__file__: m4 would expand the
                          dnl __file__ builtin into the configure.ac filename.
                          [neml2_library=`python3 -c 'import importlib.util as U, os; print(os.path.dirname(U.find_spec("neml2").origin))' 2>/dev/null`])
                    AS_IF([test "x$neml2_library" = x],
                          [AC_MSG_ERROR([Could not locate neml2. Pass --with-neml2=<path>, set NEML2_DIR, or run scripts/update_and_rebuild_neml2.sh to build and install neml2 (it prints the path to use).])])
              ])
        AC_DEFINE_UNQUOTED(NEML2_DIR, [$neml2_library], [The directory where neml2 is installed])
        AC_SUBST([neml2_library])
        AC_MSG_RESULT([NEML2 library path: $neml2_library])
      ])

AC_ARG_WITH(kokkos,
            AS_HELP_STRING([--with-kokkos@<:@=ARG@:>@],[Specify if Kokkos support should be enabled @<:@ARG=yes or cpu to disable GPU capabilities@:>@]),
            [kokkos_support=yes],
            [kokkos_support=no])

AC_ARG_WITH(kokkos-index-size,
            AS_HELP_STRING([--with-kokkos-index-size=ARG],[Specify Kokkos index size @<:@ARG=4 (default) or 8@:>@]))

AS_IF([test "x$with_kokkos_index_size" != x && test "$kokkos_support" != yes],
      [AC_MSG_ERROR([--with-kokkos-index-size requires --with-kokkos])])

AC_MSG_RESULT([configuring with kokkos support: $kokkos_support])

AS_IF([test "$kokkos_support" = yes],
      [
        AC_DEFINE(KOKKOS_ENABLED, 1, [Whether to use Kokkos-related code or not])
        AC_SUBST([kokkos_support],[true])

        AS_IF([test "x$with_kokkos_index_size" = x],
              [with_kokkos_index_size=4])

        AS_CASE([$with_kokkos_index_size],
                [4],
                [
                  AC_MSG_RESULT([Kokkos index size: 4 bytes])
                  AC_DEFINE([KOKKOS_INDEX_TYPE], [uint32_t], [Kokkos index type])
                ],
                [8],
                [
                  AC_MSG_RESULT([Kokkos index size: 8 bytes])
                  AC_DEFINE([KOKKOS_INDEX_TYPE], [uint64_t], [Kokkos index type])
                ],
                [
                  AC_MSG_ERROR([invalid value for --with-kokkos-index-size: $with_kokkos_index_size (valid values are 4 or 8)])
                ])

        AS_IF([test "$with_kokkos" = cpu],
              [
                  AC_MSG_RESULT([Kokkos GPU capabilities disabled])
                  AC_SUBST([kokkos_gpu],[false])
              ],
              [
                  AC_MSG_RESULT([Kokkos GPU capabilities enabled])
                  AC_SUBST([kokkos_gpu],[true])
              ])
      ])

AC_PATH_TOOL(PKG_CONFIG,pkg-config)
if test x$PKG_CONFIG != x; then
  AC_SUBST(LIBPNG)
  if $PKG_CONFIG --exists libpng; then
     AS_MESSAGE(checking system for libpng)
     LIBPNG_LIBS=`$PKG_CONFIG --libs libpng`
     LIBPNG_INCLUDES=`$PKG_CONFIG --cflags-only-I libpng`

     AC_DEFINE(HAVE_LIBPNG, 1, [Whether or not libpng was detected on the system])
     AC_MSG_RESULT(configuring with libpng support)
  else
     AC_MSG_RESULT(configuring without libpng support)
  fi
fi

AC_SUBST([LIBPNG_LIBS])
AC_SUBST([LIBPNG_INCLUDES])
AC_SUBST([prefix])

AC_CONFIG_FILES(conf_vars.mk)

#AC_CHECK_LIB([png], [png_create_write_struct_2])

AC_OUTPUT

AC_MSG_RESULT(---------------------------------------------)
AC_MSG_RESULT(--------- Done Configuring MOOSE ------------)
AC_MSG_RESULT(---------------------------------------------)
