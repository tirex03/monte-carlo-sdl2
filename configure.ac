AC_INIT([montecarlo], [0.0.2])
AM_INIT_AUTOMAKE([-Wall foreign])
AC_CONFIG_SRCDIR([src/main.c])
AC_PROG_CC
AC_CHECK_HEADERS([SDL2/SDL.h sodium.h])
AC_CHECK_LIB(SDL2, SDL_Init)
AC_CHECK_LIB(sodium, sodium_init)
AC_OUTPUT([Makefile src/Makefile])