dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([SCREAM], 0.1, stephan.mennicke@gmail.com, scream)
# Staged ConfiguRation of fEAture Models

AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS([src/config.h])

AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE(gnits)

# Check Standard C Programming Tools
AC_PROG_CXX
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_LEX
AC_PROG_RANLIB
AC_PROG_YACC

# Check for necessary programs
AC_PATH_PROG([LOLA_MODELCHECKING], [lola-modelchecking], not found)
AC_PATH_PROG([LOLA_DEADTRANSITION], [lola-deadtransition], not found)

AC_CHECK_PROG(DOT, dot, dot)
AC_CHECK_PROG(GENGETOPT, gengetopt, gengetopt)

# Check for headers
AC_HEADER_STDC
#AC_CHECK_HEADER([libxml++-2.6/libxml++/libxml++.h])

# Check for functions
AC_CHECK_FUNCS([popen pclose],,AC_MSG_ERROR([cannot use required functions]))

AC_CONFIG_FILES(Makefile libs/pnapi/Makefile src/Makefile tests/Makefile)
AC_OUTPUT

