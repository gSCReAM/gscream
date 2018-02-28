build send with:
g++ -Wall send.cpp -o send $(pkg-config --cflags --libs gstreamer-1.0)

build recv with:
g++ -Wall recv.cpp -o recv $(pkg-config --cflags --libs gstreamer-1.0)


run send:
./send /dev/video0 127.0.0.1 5200

run recv:
./recv 5200


both send and recv can generate .dot files for pipeline visualisation

run with GST_DEBUG_DUMP_DOT_DIR=. for both send and recv and the .dot files will
be generated in the working dir.
ex: GST_DEBUG_DUMP_DOT_DIR=. ./send /dev/video0 127.0.0.1 5200

to convert .dot file to pdf run
dot -Tpdf filename.dot > filename.pdf

to convert .dot file to png run
dot -Tpng filename.dot > filename.png

graphviz is required to convert the dot files.


to create dot files from pipeline for debug run
GST_DEBUG_DUMP_DOT_DIR=. pipeline...
