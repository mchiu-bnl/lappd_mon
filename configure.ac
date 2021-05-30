AC_INIT(LAPPDMon,0.01)
AC_CONFIG_SRCDIR(configure.ac)

AM_INIT_AUTOMAKE(LAPPDMon, 0.01)

AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CXX(CC g++)
AC_ENABLE_STATIC(no)
AC_PROG_LIBTOOL

ROOTLIBS=`root-config --libs`
AC_SUBST(ROOTLIBS)

ROOTGLIBS=`root-config --glibs`
AC_SUBST(ROOTGLIBS)

if test $ac_cv_prog_gxx = yes; then
   CXXFLAGS="$CXXFLAGS -std=c++11 -g -O2 -Wall -Werror"
fi

AC_OUTPUT(Makefile)

