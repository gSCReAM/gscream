# Note: plugindir is set in configure

##############################################################################
# TODO: change libgstplugin.la to something else, e.g. libmysomething.la     #
##############################################################################
plugin_LTLIBRARIES = libgstgscreamrx.la

##############################################################################
# TODO: for the next set of variables, name the prefix if you named the .la, #
#  e.g. libmysomething.la => libmysomething_la_SOURCES                       #
#                            libmysomething_la_CFLAGS                        #
#                            libmysomething_la_LIBADD                        #
#                            libmysomething_la_LDFLAGS                       #
##############################################################################

## Plugin 1

# sources used to compile this plug-in
libgstgscreamrx_la_SOURCES = gstgscreamrx.c gstgscreamrx.h

# compiler and linker flags used to compile this plugin, set in configure.ac
libgstgscreamrx_la_CFLAGS = $(GST_CFLAGS)
libgstgscreamrx_la_LIBADD = $(GST_LIBS)
libgstgscreamrx_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstgscreamrx_la_LIBTOOLFLAGS = --tag=disable-static


# headers we need but don't want installed
noinst_HEADERS = gstgscreamrx.h
