### Build
 * gscream_app_tx

    ```$ g++ -Wall gscream_app_tx.cpp -o gscream_app_tx $(pkg-config --cflags --libs gstreamer-1.0)```

 * gscream_app_rx

  ```$ g++ -Wall gscream_app_rx.cpp -o gscream_app_rx $(pkg-config --cflags --libs gstreamer-1.0)```

### Running application
 * gscream_app_tx

 ```$ ./gscream_app_tx /dev/video0 127.0.0.1 5200```

 * gscream_app_rx

 ```$ ./gscream_app_rx 5200```

### Debug
Both __gscream_app_tx__ and __gscream_app_rx__ can generate __.dot__ files for pipeline visualisation

#### Dependencies
To generate __.dot__ files __graphviz__ needs to be installed

#### Generate _dot_ file while running
run with ```GST_DEBUG_DUMP_DOT_DIR=.``` for both __gscream_app_tx__ and __gscream_app_rx__ to generate the __.dot__ files in the working dir.

```
$ GST_DEBUG_DUMP_DOT_DIR=. ./gscream_app_tx /dev/video0 127.0.0.1 5200
$ GST_DEBUG_DUMP_DOT_DIR=. ./gscream_app_rx 5200

```

#### Convert _dot_ file to _pdf_ or _png_
* _.dot_ file to __pdf__

  ```$ dot -Tpdf filename.dot > filename.pdf```

* _.dot_ file to __png__

  ```$ dot -Tpng filename.dot > filename.png```




to create dot files for a terminal run pipeline for debug
```$ GST_DEBUG_DUMP_DOT_DIR=. pipeline```



#### Other debug
For a normal text debug in the terminal one can debug
* _Errors_ with

  ```$ GST_DEBUG=*:LOG ./appfilename 2>&1 | grep -i error```

* _Flow_ with

  ```$ GST_DEBUG=*:LOG ./appfilename 2>&1 | grep -i flow```

* _Warnings_ with

  ```$ GST_DEBUG=*:LOG ./appfilename 2>&1 | grep -i warning```

* _Info_ with

  ```$ GST_DEBUG=*:LOG ./appfilename 2>&1 | grep -i info```

* _Debug_ with

  ```$ GST_DEBUG=*:LOG ./appfilename 2>&1 | grep -i debug```

* _Log_ with

  ```$ GST_DEBUG=*:LOG ./appfilename 2>&1 | grep -i log```

* _Trace_ with

  ```$ GST_DEBUG=*:LOG ./appfilename 2>&1 | grep -i trace```

To save the output to __file__ instead use something like

```$ GST_DEBUG_FILE="filename" GST_DEBUG_NO_COLOR=1 GST_DEBUG=*:LOG ./appfilename 5200 2>&1 | grep -i option from above```
