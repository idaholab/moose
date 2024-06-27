AC_INIT([moose], [0.9.0], [https://github.com/idaholab/moose/discussions], [moose], [https://mooseframework.org])
AC_PREREQ([2.69])

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
AC_CONFIG_HEADER(framework/include/base/MooseConfig.h.tmp:framework/include/base/MooseConfig.h.in)
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
                          [libtorch_library="$ac_pwd/framework/contrib/libtorch"])
              ])
        AC_DEFINE_UNQUOTED(LIBTORCH_DIR, [$libtorch_library], [The directory where libtorch is installed])
        AC_SUBST([libtorch_library])
        AC_MSG_RESULT([Libtorch library path: $libtorch_library])
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
