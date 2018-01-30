# Installing SCReAM plugin

Within gst-scream folder, use ```./autogen.sh```, this should generate all the executable files within .libs

Do ```make```.

If no errors, remember to set the GST environment path so it points on the gst-scream folder by exporting it.

For example, we use:

```export GST_PLUGIN_PATH=$HOME/gst/master/gst-template/gst-scream/src/.libs:$GST_PLUGIN_PATH```

To inspect:

```gst-inspect-1.0 scream```

To run with empty pipeline:

```gst-launch-1.0 videotestsrc ! scream ! autovideosrc```

If it's installed properly a videotestsrc will show up.
