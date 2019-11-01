AC_INIT([moose], [0.9.0], [moose-users@googlegroups.com], [moose], [https://mooseframework.org])
AC_PREREQ([2.69])

# Infer the source directory as the path to the ./configure script
srcdir=`dirname $0`
top_srcdir=`dirname $0`

AC_MSG_RESULT(---------------------------------------------)
AC_MSG_RESULT(----------- Configuring MOOSE ---------------)
AC_MSG_RESULT(---------------------------------------------)

AC_CONFIG_MACRO_DIR([m4])

# Create a temporary file with e.g. SPARSE_AD
AC_CONFIG_HEADER(include/base/MooseConfig.h.tmp:include/base/MooseConfig.h.in)
# Prefixes everything in the temporary file with MOOSE_, e.g. MOOSE_SPARSE_AD
AX_PREFIX_CONFIG_H(include/base/MooseConfig.h,MOOSE,include/base/MooseConfig.h.tmp)

AC_ARG_WITH(derivative-type,
            AS_HELP_STRING([--with-derivative-type=sparse,nonsparse],[Specify the derivative type to use]),
            [derivative_type="$withval"],
            [derivative_type=nonsparse])

AS_CASE("$derivative_type",
        [sparse], [AC_DEFINE(SPARSE_AD, 1, [Whether to use a sparse derivative type])],
        [nonsparse], [],
        [AC_MSG_ERROR(bad value "$derivative_type" for --with-derivative-type)])

AC_MSG_RESULT([configuring with derivative type $derivative_type])

AC_ARG_WITH(derivative-size,
            AS_HELP_STRING([--with-derivative-size=<n>], [Specify the size of the derivative backing array]),
            [derivative_size="$withval"],
            [derivative_size=50])

AC_DEFINE_UNQUOTED(AD_MAX_DOFS_PER_ELEM, [$derivative_size], [The size of the derivative backing array])

AC_MSG_RESULT([configuring with derivative backing array size of $derivative_size])

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

AC_CONFIG_FILES(conf_vars.mk)

#AC_CHECK_LIB([png], [png_create_write_struct_2])

AC_OUTPUT

AC_MSG_RESULT(---------------------------------------------)
AC_MSG_RESULT(--------- Done Configuring MOOSE ------------)
AC_MSG_RESULT(---------------------------------------------)
