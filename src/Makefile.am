# Note: screamdir is set in configure

##############################################################################
# TODO: change libgstscream.la to something else, e.g. libmysomething.la     #
##############################################################################
plugin_LTLIBRARIES = libgstscream.la

##############################################################################
# TODO: for the next set of variables, name the prefix if you named the .la, #
#  e.g. libmysomething.la => libmysomething_la_SOURCES                       #
#                            libmysomething_la_CFLAGS                        #
#                            libmysomething_la_LIBADD                        #
#                            libmysomething_la_LDFLAGS                       #
##############################################################################

# sources used to compile this plug-in
libgstscream_la_SOURCES = gstscream.c gstscream.h

# compiler and linker flags used to compile this scream, set in configure.ac
libgstscream_la_CFLAGS = $(GST_CFLAGS)
libgstscream_la_LIBADD = $(GST_LIBS)
libgstscream_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstscream_la_LIBTOOLFLAGS = --tag=disable-static

# headers we need but don't want installed
noinst_HEADERS = gstscream.h
