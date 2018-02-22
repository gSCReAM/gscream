/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2018  <<user@hostname.org>>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


/*
    ---- Nuvarande stadie ----


Uppgift 1: Vi vill kunna skicka RTC/RTCP paket samt manipulera dataströmmen för att implementera SCReAM.

Vår approach: skapa ett bin element som har rtp/rtcp element vilket vi vill koppla gstreamers pipeline till.
   - Vi kan skapa element och binda samman dom i en intern pipeline (sett i bind_communication).
   - Tanken är att vi ska koppla gstreamers pipeline till denna pipeline.
   - Tanken är även att denna interna pipeline har udp sockets/udp pads för att ta emot/skicka data.
   - Vi har inte börjat kolla på ghostpads ännu, men fick höra i ett möte att detta är hur det bör fungera, tanken är att implementera detta.

Vårt problem:
  - Vid gst_element_link_pads (scream, "src", rtpbin, "recv_rtcp_sink_0"); (ungefär rad 350) är vårt problem att koppla samman scream src pad
  med rtpbins sink pad.
  - Vi vet inte riktigt vart som padsen ska bli kopplade, inte heller vart som dataströmmen ska "matas in" i denna pipeline.

Uppgift 2: Implementera någon slags "RTP queue" som ska matas in i screamtx.registerNewStream("RTP queue", ...)

Vår approach: Följa Ingemars testapplikation sett i scream_transmitter.cpp och sedan konvertera trådar och mutex till gst_task

Vårt problem:
  - Vi har inte försökt så mycket än. Så det mesta.

Prio för tillfället: Uppgift 1. Huvudproblemet är just att koppla pipelinen till individuella element. Kan vi fixa det så kan vi förmodligen fixa
resten själva. 

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gst/gst.h>

#include "gstgscreamtx.h"

#include "ScreamTx.h"

GST_DEBUG_CATEGORY_STATIC (gst_g_scream_tx_debug);
#define GST_CAT_DEFAULT gst_g_scream_tx_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_SILENT
};
#define DEST_HOST "127.0.0.1"

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */

/* GObject vmethod implementations */

/* initialize the gscreamtx's class */
static void
gst_g_scream_tx_clas_init (GstgScreamTxClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  gobject_class->set_property = gst_g_scream_tx_set_property;
  gobject_class->get_property = gst_g_scream_tx_get_property;

  g_object_class_install_property (gobject_class, PROP_SILENT,
      g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?",
          FALSE, G_PARAM_READWRITE));

  gst_element_class_set_details_simple(gstelement_class,
    "gScreamTx",
    "FIXME:Generic",
    "FIXME:Generic Template Element",
    " <<user@hostname.org>>");

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&src_factory));
  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&sink_factory));
  //bind_communication (GST_ELEMENT (scream));

}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static void
gst_g_scream_tx_init (GstgScreamTx * scream)
{
  ScreamTx *screamTx = new ScreamTx();
  screamTx->assertWorking();

  scream->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink");

  gst_pad_set_event_function (scream->sinkpad,
                              GST_DEBUG_FUNCPTR(gst_g_scream_tx_sink_event));

  gst_pad_set_chain_function (scream->sinkpad,
                              GST_DEBUG_FUNCPTR(gst_g_scream_tx_chain));

  GST_PAD_SET_PROXY_CAPS (scream->sinkpad);
  gst_element_add_pad (GST_ELEMENT (scream), scream->sinkpad);

  scream->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
  GST_PAD_SET_PROXY_CAPS (scream->srcpad);
  gst_element_add_pad (GST_ELEMENT (scream), scream->srcpad);

  scream->silent = TRUE;
}

static void
gst_g_scream_tx_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstgScreamTx *scream = GST_GSCREAMTX (object);

  switch (prop_id) {
    case PROP_SILENT:
      scream->silent = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_g_scream_tx_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstgScreamTx *scream = GST_GSCREAMTX (object);

  switch (prop_id) {
    case PROP_SILENT:
      g_value_set_boolean (value, scream->silent);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* GstElement vmethod implementations */

/* this function handles sink events */
static gboolean
gst_g_scream_tx_sink_event (GstPad * pad, GstObject * parent, GstEvent * event)
{
  GstgScreamTx *scream;
  gboolean ret;

  scream = GST_GSCREAMTX (parent);

  GST_LOG_OBJECT (scream, "Received %s event: %", GST_PTR_FORMAT,
      GST_EVENT_TYPE_NAME (event), event);

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_CAPS:
    {
      GstCaps * caps;

      gst_event_parse_caps (event, &caps);
      /* do something with the caps */

      /* and forward */
      ret = gst_pad_event_default (pad, parent, event);
      break;
    }
    default:
      ret = gst_pad_event_default (pad, parent, event);
      break;
  }
  return ret;
}

/* chain function
 * this function does the actual processing
 */
static GstFlowReturn
gst_g_scream_tx_chain (GstPad * pad, GstObject * parent, GstBuffer * buf)
{
  GstgScreamTx *scream = GST_GSCREAMTX (parent);

  scream = GST_GSCREAMTX (parent);

  if (scream->silent == FALSE)
  g_print ("Have data of size %" G_GSIZE_FORMAT" bytes!\n",
      gst_buffer_get_size (buf));

  //Här bör buf, som bör vara i rtpform, matas in i rtpqueue som huvuduppgift.

  return gst_pad_push (scream->srcpad, buf);
  // Detta ^ bör ske i en annan tråd som kontinuerligt tar data ifrån rtpqueue och skickar datat till udpsocket.
}


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
gscreamtx_init (GstPlugin * gscreamtx)
{
  /* debug category for fltering log messages
   *
   * exchange the string 'Template gscreamtx' with your description
   */
  GST_DEBUG_CATEGORY_INIT (gst_g_scream_tx_debug, "gscreamtx",
      0, "Template gscreamtx");

  return gst_element_register (gscreamtx, "gscreamtx", GST_RANK_NONE,
      GST_TYPE_GSCREAMTX);
}


/**
* ------- GSCReAM FUNCTIONS -------
*/

static GstElement *
bind_communication (GstElement * scream)
{
  GstElement *pipeline, *rtpbin, *audiosrc, *audio_encoder,
      *audio_rtppayloader, *sendrtp_udpsink,
      *send_rtcp_udpsink, *sendrtcp_funnel, *sendrtp_funnel;
  GstElement *videosrc, *video_rtppayloader, *time_overlay;
  gint rtp_udp_port = 5001;
  gint rtcp_udp_port = 5002;
  gint recv_audio_rtcp_port = 5003;
  gint recv_video_rtcp_port = 5004;
  GstElement *audio_rtcp_udpsrc, *video_rtcp_udpsrc;

  pipeline = gst_pipeline_new (NULL);

  rtpbin = gst_element_factory_make ("rtpbin", NULL);

  audiosrc = gst_element_factory_make ("audiotestsrc", NULL);
  g_object_set (audiosrc, "is-live", TRUE, NULL);
  audio_encoder = gst_element_factory_make ("alawenc", NULL);
  audio_rtppayloader = gst_element_factory_make ("rtppcmapay", NULL);
  g_object_set (audio_rtppayloader, "pt", 96, NULL);

  videosrc = gst_element_factory_make ("videotestsrc", NULL);
  g_object_set (videosrc, "is-live", TRUE, NULL);
  time_overlay = gst_element_factory_make ("timeoverlay", NULL);
  video_rtppayloader = gst_element_factory_make ("rtpvrawpay", NULL);
  g_object_set (video_rtppayloader, "pt", 100, NULL);

  /* muxed rtcp */
  sendrtcp_funnel = gst_element_factory_make ("funnel", "send_rtcp_funnel");
  send_rtcp_udpsink = gst_element_factory_make ("udpsink", NULL);
  g_object_set (send_rtcp_udpsink, "host", "127.0.0.1", NULL);
  g_object_set (send_rtcp_udpsink, "port", rtcp_udp_port, NULL);
  g_object_set (send_rtcp_udpsink, "sync", FALSE, NULL);
  g_object_set (send_rtcp_udpsink, "async", FALSE, NULL);

  /* outgoing bundled stream */
  sendrtp_funnel = gst_element_factory_make ("funnel", "send_rtp_funnel");
  sendrtp_udpsink = gst_element_factory_make ("udpsink", NULL);
  g_object_set (sendrtp_udpsink, "host", "127.0.0.1", NULL);
  g_object_set (sendrtp_udpsink, "port", rtp_udp_port, NULL);
  g_object_set (sendrtp_udpsink, "sync", FALSE, NULL);
  g_object_set (sendrtp_udpsink, "async", FALSE, NULL);

  gst_bin_add_many (GST_BIN (pipeline), rtpbin, audiosrc, audio_encoder,
      audio_rtppayloader, sendrtp_udpsink, send_rtcp_udpsink,
      sendrtp_funnel, sendrtcp_funnel, videosrc, video_rtppayloader, NULL);

  if (time_overlay)
    gst_bin_add (GST_BIN (pipeline), time_overlay);

  gst_element_link_many (audiosrc, audio_encoder, audio_rtppayloader, NULL);
  gst_element_link_pads (audio_rtppayloader, "src", rtpbin, "send_rtp_sink_0");

  if (time_overlay) {
    gst_element_link_many (videosrc, time_overlay, video_rtppayloader, NULL);
  } else {
    gst_element_link (videosrc, video_rtppayloader);
  }

  gst_element_link_pads (video_rtppayloader, "src", rtpbin, "send_rtp_sink_1");

  gst_element_link_pads (sendrtp_funnel, "src", sendrtp_udpsink, "sink");
  gst_element_link_pads (rtpbin, "send_rtp_src_0", sendrtp_funnel, "sink_%u");
  gst_element_link_pads (rtpbin, "send_rtp_src_1", sendrtp_funnel, "sink_%u");
  gst_element_link_pads (sendrtcp_funnel, "src", send_rtcp_udpsink, "sink");
  gst_element_link_pads (rtpbin, "send_rtcp_src_0", sendrtcp_funnel, "sink_%u");
  gst_element_link_pads (rtpbin, "send_rtcp_src_1", sendrtcp_funnel, "sink_%u");

  audio_rtcp_udpsrc = gst_element_factory_make ("udpsrc", NULL);
  g_object_set (audio_rtcp_udpsrc, "port", recv_audio_rtcp_port, NULL);

  video_rtcp_udpsrc = gst_element_factory_make ("udpsrc", NULL);
  g_object_set (video_rtcp_udpsrc, "port", recv_video_rtcp_port, NULL);


  gst_bin_add_many (GST_BIN (pipeline), audio_rtcp_udpsrc, video_rtcp_udpsrc,
      NULL);
  gst_element_link_pads (audio_rtcp_udpsrc, "src", rtpbin, "recv_rtcp_sink_0");
  gst_element_link_pads (video_rtcp_udpsrc, "src", rtpbin, "recv_rtcp_sink_1");


  gst_element_link_pads (scream, "src", rtpbin, "recv_rtcp_sink_0");

  return pipeline;
}


/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "myfirstgscreamtx"
#endif

/* gstreamer looks for this structure to register gscreamtxs
 *
 * exchange the string 'Template gscreamtx' with your gscreamtx description
 */
 GST_PLUGIN_DEFINE(GST_VERSION_MAJOR, GST_VERSION_MINOR, scream,
                   "Congestion control using the SCReAM algorithm. Developed by "
                   "LTU students for Ericsson.",
                   gscreamtx_init, VERSION, "LGPL", "GStreamer",
                   "http://gstreamer.net/")
