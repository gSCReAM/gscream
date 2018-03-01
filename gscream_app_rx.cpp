#include <iostream>
#include <string>
#include <gst/gst.h>
#include <glib.h>

/*
program runs the following pipeline (currently not with queue)
gst-launch-1.0 udpsrc port=n caps="application/x-rtp, media=video, encoding-name=H264" !
  rtph264depay ! queue ! decodebin ! videoconvert ! autovideosink

*/
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

static void on_pad_added (GstElement *element,
              GstPad     *pad,
              gpointer    data){
  GstPad *sinkpad;
  GstElement *videoconvert = (GstElement *) data;

  /* We can now link this pad with the vorbis-decoder sink pad */
  g_print ("Dynamic pad created, linking decodebin and videoconvert\n");

  sinkpad = gst_element_get_static_pad (videoconvert, "sink");

  gst_pad_link (pad, sinkpad);
  gst_object_unref (sinkpad);
}

int main (int argc, char *argv[])
{
  std::printf("going for video on port: %s\n", argv[1]);

  GMainLoop *loop;

  GstElement *pipeline, *udpsrc, *rtpdepay, *decodebin, *videoconvert, *autovideosink;
  GstBus *bus;
  guint bus_watch_id;
  GstCaps *udpcaps;

  /* Initialisation */
  gst_init (&argc, &argv);

  loop = g_main_loop_new (NULL, FALSE);

  /* Create gstreamer elements */
  pipeline        = gst_pipeline_new ("video-pipeline");
  udpsrc          = gst_element_factory_make("udpsrc",          "udpsrc");
  rtpdepay        = gst_element_factory_make("rtph264depay",    "rtph264depay0");
  decodebin       = gst_element_factory_make ("decodebin",      "decodebin");
  videoconvert    = gst_element_factory_make ("videoconvert",   "videoconvert");
  autovideosink   = gst_element_factory_make ("autovideosink",  "videosink");

  if (!pipeline || !udpsrc || !rtpdepay || !decodebin || !videoconvert || !autovideosink) {
    g_printerr ("One element could not be created. Exiting.\n");
    return -1;
  }

  udpcaps = gst_caps_new_simple ("application/x-rtp",
    "media", G_TYPE_STRING,"video",
    "encoding-name",G_TYPE_STRING,"H264",
    NULL);
  g_object_set(G_OBJECT(udpsrc), "port", std::atoi(argv[1]),"caps", udpcaps, NULL);
  //gst_object_unref(udpcaps);


  /* Set up the pipeline */

  /* we add a message handler */
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);


  /* we add all elements into the pipeline */

  gst_bin_add_many (GST_BIN (pipeline),
                    udpsrc, rtpdepay, decodebin, videoconvert, autovideosink, NULL);

  /* we link the elements together */
  /* videotestsrcm -> autovideosink */

  if(!gst_element_link_many (udpsrc, rtpdepay, decodebin, NULL)){
    g_error("Could not link on ore more of elements udpsrc, rtpdepay decodebin");
    return -1;
  }

  if(!gst_element_link_many (videoconvert, autovideosink, NULL)){
    g_error("Could not link elements: videoconvert, autovideosink");
    return -1;
  }

  if(!g_signal_connect (decodebin, "pad-added", G_CALLBACK (on_pad_added), videoconvert)){
    g_error("Could not create needed pad's between decodebin and videoconvert");
    return -1;
  }

  GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "recv-pipeline-bf-playing");

  /* Set the pipeline to "playing" state*/
  g_print ("Now set pipeline in state playing\n");
  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "recv-pipeline-af-playing-bf-running");

  /* Iterate */
  g_print ("Running...\n");
  g_main_loop_run (loop);
  GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "recv-pipeline-af-running");

  /* Out of the main loop, clean up nicely */
  g_print ("Returned, stopping playback\n");
  gst_element_set_state (pipeline, GST_STATE_NULL);
  GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "recv-pipeline-af-stop");

  g_print ("Deleting pipeline\n");
  gst_object_unref (GST_OBJECT (pipeline));
  g_source_remove (bus_watch_id);
  g_main_loop_unref (loop);

  return 0;
}
