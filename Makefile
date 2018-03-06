CXX= g++
CXXFLAGS= -Wall
PKGCONFIG= pkg-config
PACKAGES= gstreamer-1.0
LIBS:= $(shell $(PKGCONFIG) --cflags --libs $(PACKAGES))
EXECNAME= gscream_app_rx gscream_app_tx
CXXSOURCES= gscream_app_rx.cpp gscream_app_tx.cpp
CXXOBJECTS= $(patsubst %.cpp, %.o, $(CXXSOURCES))

all: send recv

send: $(CXXOBJECTS)
	$(CXX) $(CXXFLAGS) gscream_app_tx.o -o gscream_app_tx $(LIBS)

recv: $(CXXOBJECTS)
	$(CXX) $(CXXFLAGS) gscream_app_rx.o -o gscream_app_rx $(LIBS)

$(CXXOBJECTS): $(CXXSOURCES)
	$(CXX) $(CXXFLAGS) -c $? $(LIBS)

clean:
	rm *.o

uninstall:
	rm gscream_app_rx gscream_app_tx
