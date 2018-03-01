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

  std::string pavideosrc = "device= ";
  pavideosrc.append(argv[1]);
  const char *charvideosrc = pavideosrc.c_str();
  std::printf("going for video at: %s\n",charvideosrc);
  const char *srcport="port=5000";
  GMainLoop *loop;

  GstElement *pipeline, /*videotestsrc,*/ *udpsrcm, *rtpdepaym, *queuem, *decodebinm, *videoconvertm, *autovideosinkm;
  GstBus *bus;
  guint bus_watch_id;

  /* Initialisation */
  gst_init (&argc, &argv);

  loop = g_main_loop_new (NULL, FALSE);


  //gst-launch-1.0 udpsrc port=5200 caps="application/x-rtp, media=video, encoding-name=H264" ! rtph264depay ! queue ! decodebin ! videoconvert ! autovideosink
  //std::string tt = "device="+argv[1];
  /* Create gstreamer elements */
  pipeline = gst_pipeline_new ("videotest-pipeline");
  //videotestsrc   = gst_element_factory_make ("videotestsrc", "testsource");
  //videotestsrc = gst_element_factory_make ("v4l2src", charvideosrc);
  udpsrcm = gst_element_factory_make("udpsrc",srcport);
  rtpdepaym = gst_element_factory_make("rtph264depay", "rtph264depay");
  queuem = gst_element_factory_make ("queue", "queue");
  decodebinm = gst_element_factory_make ("decodebin", "decodebin");
  videoconvertm = gst_element_factory_make ("videoconvert", "videoconvert");

  autovideosinkm = gst_element_factory_make ("autovideosink", "videosink");
// gst-launch-1.0 udpsrc port=5200 caps="application/x-rtp, media=video, encoding-name=H264" ! rtph264depay ! queue ! decodebin ! videoconvert ! autovideosink

  /*if (!pipeline || !videotestsrc || !autovideosinkm) {
    g_printerr ("One element could not be created. Exiting.\n");
    return -1;
  }*/
  if (!pipeline || !udpsrcm || !rtpdepaym || !decodebinm || !videoconvertm || !autovideosinkm) {
    g_printerr ("One element could not be created. Exiting.\n");
    return -1;
  }
  //g_object_set (source, "device", argv[1], NULL);
  //g_object_set(G_OBJECT(udpsrcm), "port", 5200, NULL);
  g_object_set (G_OBJECT (udpsrcm), "port", 5200, "caps",
        gst_caps_new_simple ("application/x-rtp",
            "media", G_TYPE_STRING,"video",
            "encoding-name",G_TYPE_STRING,"H264",
            NULL), NULL);
    //g_object_set (G_OBJECT (udpsrcm), "port", 5200, "caps",gst_caps_new_simple ("application/x-rtp",NULL,NULL), NULL);
  //g_object_set (G_OBJECT (udpsrcm), "port", 5200, NULL);



  /* Set up the pipeline */

  /* we add a message handler */
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);


  /* we add all elements into the pipeline */
  //gst_bin_add_many (GST_BIN (pipeline),
  //                  videotestsrc, autovideosinkm, NULL);

  //gst_bin_add_many (GST_BIN (pipeline),
  //                  udpsrcm, autovideosinkm, NULL);

// gst-launch-1.0 udpsrc port=5200 caps="application/x-rtp, media=video, encoding-name=H264" ! rtph264depay ! decodebin ! videoconvert ! autovideosin
  gst_bin_add_many (GST_BIN (pipeline),
                    udpsrcm, rtpdepaym, decodebinm, videoconvertm, autovideosinkm, NULL);

  /* we link the elements together */
  /* videotestsrc -> autovideosinkm */
  //gst_element_link (videotestsrc, autovideosinkm);
  //gst_element_link (udpsrcm, autovideosinkm);

  gst_element_link_many (udpsrcm, rtpdepaym, decodebinm, videoconvertm, autovideosinkm, NULL);

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
