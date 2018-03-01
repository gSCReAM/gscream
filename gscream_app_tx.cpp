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
  g_printerr("Hej");
  std::string pavideosrc = "";
  if(argv[1] != NULL){
    pavideosrc.append(argv[1]);
    const char *charvideosrc = pavideosrc.c_str();
    std::printf("going for video at: %s\n",charvideosrc);
  }else{
    std::printf("No video source found.");
  }
  GMainLoop *loop;

  GstElement *pipeline, *videotestsrc, *udpsrc, *videoconvert, *videoencode, *videopayload, *autovideosink, *gscreamtx;
  GstBus *bus;
  guint bus_watch_id;

  /* Initialisation */
  //gst_init (&argc, &argv);

  loop = g_main_loop_new (NULL, FALSE);

  /* Create gstreamer elements */
  pipeline = gst_pipeline_new ("videotest-pipeline");
  //videotestsrc   = gst_element_factory_make ("videotestsrc", "testsource");

  /*Main imports*/
  videotestsrc    = gst_element_factory_make ("v4l2src",        "video4linux2");
  videoconvert    = gst_element_factory_make ("videoconvert",   "convert");
  videoencode     = gst_element_factory_make ("x264enc",        "encode");
  videopayload    = gst_element_factory_make ("rtph264pay",     "payload");
  gscreamtx       = gst_element_factory_make ("gscreamtx",      "gscream-transmit-cc");
  udpsrc          = gst_element_factory_make ("udpsink",        "udpsink");

  /* Optional imports*/
  autovideosink  = gst_element_factory_make ("autovideosink",  "videosink");
  g_object_set(G_OBJECT(udpsrc), "host", "127.0.0.1", "port", 5200, NULL);

  //gst-launch-1.0 -v videotestsrc ! videoconvert ! x264enc ! rtph264pay ! udpsink host=127.0.0.1 port=5200
  if (!pipeline || !videotestsrc || !videoconvert || !videoencode || !videopayload || !autovideosink) {
    g_printerr ("One element could not be created. Exiting.\n");
    return -1;
  }

  /* Set up the pipeline */

  /* we add a message handler */
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);


  /* we add all elements into the pipeline */

  gst_bin_add_many (GST_BIN (pipeline),
                    videotestsrc, videoconvert, videoencode, videopayload, autovideosink, NULL);

  /* we link the elements together */
  gst_element_link_many (videotestsrc, videoconvert, videoencode, videopayload, autovideosink, NULL);

  /* Set the pipeline to "playing" state*/
  g_print ("Now set pipeline in state playing\n");
  gst_element_set_state (pipeline, GST_STATE_PLAYING);


  /* Iterate */
  g_print ("Running...\n");
  g_main_loop_run (loop);


  /* Out of the main loop, clean up nicely */
  g_print ("Returned, stopping playback\n");
  gst_element_set_state (pipeline, GST_STATE_NULL);

  g_print ("Deleting pipeline\n");
  gst_object_unref (GST_OBJECT (pipeline));
  g_source_remove (bus_watch_id);
  g_main_loop_unref (loop);

  return 0;
}
