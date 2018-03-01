#include <iostream>
#include <string>
#include <gst/gst.h>
#include <glib.h>


static gboolean bus_call (GstBus *bus, GstMessage *msg, gpointer data)
{
  GMainLoop *loop = (GMainLoop *) data;

  switch (GST_MESSAGE_TYPE (msg)) {

    case GST_MESSAGE_EOS:
      g_print ("End of stream\n");
      g_main_loop_quit (loop);
      break;

    case GST_MESSAGE_ERROR: {
      gchar  *debug;
      GError *error;

      gst_message_parse_error (msg, &error, &debug);
      g_free (debug);

      g_printerr ("Error: %s\n", error->message);
      g_error_free (error);

      g_main_loop_quit (loop);
      break;
    }
    default:
      break;
  }

  return TRUE;
}



int main (int argc, char *argv[])
{
  g_printerr(" \n --- Initializing gscream_app_tx \n");

  if(argc < 4){
    std::cout << "Usage: " << argv[0] << " videosrc ip(recieving) port" << std::endl;
    std::cout << "Example: " << argv[0] << " /dev/video0 127.0.0.1 5000   sends the video stream to localhost on port 5000" << std::endl;
    return -1;
  }

  std::printf("Sending video from source: %s to ip: %s port %i\n",argv[1], argv[2],std::atoi(argv[3]));


  GMainLoop *loop;
  GstElement *pipeline, *videosrc, *udpsrc, *videoconvert, *capsfilter,
   *videoencode, *videopayload, *autovideosink, *gscreamtx;
  GstBus *bus;
  guint bus_watch_id;
  GstCaps *capsfiltercaps;

  /* Initialisation */
  gst_init (&argc, &argv);

  loop = g_main_loop_new (NULL, FALSE);

  /* Create gstreamer elements */
  pipeline = gst_pipeline_new ("videotest-pipeline");

  /*Main imports*/
  videosrc        = gst_element_factory_make ("v4l2src",        "video4linux2");
  videoconvert    = gst_element_factory_make ("videoconvert",   "convert");
  capsfilter      = gst_element_factory_make ("capsfilter",     "capsfilter");
  videoencode     = gst_element_factory_make ("x264enc",        "encode");
  videopayload    = gst_element_factory_make ("rtph264pay",     "payload");
  //gscreamtx       = gst_element_factory_make ("gscreamtx",      "gscream-transmit-cc");
  udpsrc          = gst_element_factory_make ("udpsink",        "udpsink");

  /* Optional imports*/
  autovideosink  = gst_element_factory_make ("autovideosink",  "videosink");

  if (!pipeline || !videosrc || !videoconvert || !capsfilter || !videoencode || !videopayload || !autovideosink) {
    g_printerr ("One element could not be created. Exiting.\n");
    return -1;
  }

  /* Setting element properties*/
  g_object_set  (G_OBJECT(videosrc), "device", argv[1], NULL);
  g_object_set  (G_OBJECT(udpsrc), "host", argv[2], "port", std::atoi(argv[3]), NULL);

  capsfiltercaps = gst_caps_new_simple ("video/x-raw",
      "width", G_TYPE_INT, 1280,
      "height", G_TYPE_INT, 720,
      NULL);
  g_object_set (G_OBJECT(capsfilter), "caps",  capsfiltercaps, NULL);




  /* Set up the pipeline */

  /* we add a message handler */
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);


  /* we add all elements into the pipeline */
  gst_bin_add_many (GST_BIN (pipeline),
                    videosrc, capsfilter, videoconvert, videoencode, videopayload, udpsrc, NULL);

  /* we link the elements together */
  gst_element_link_many (videosrc, capsfilter, videoconvert, videoencode, videopayload, udpsrc, NULL);

  /* Set the pipeline to "playing" state*/
  g_print ("Now set pipeline in state playing\n");
  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "send-pipeline-af-playing-bf-running");

  /* Iterate */
  g_print ("Running...\n");
  g_main_loop_run (loop);


  /* Out of the main loop, clean up nicely */
  g_print ("Returned, stopping playback\n");
  gst_element_set_state (pipeline, GST_STATE_NULL);
  GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "send-pipeline-af-stopping");

  g_print ("Deleting pipeline\n");
  gst_object_unref (GST_OBJECT (pipeline));
  g_source_remove (bus_watch_id);
  g_main_loop_unref (loop);

  return 0;
}
