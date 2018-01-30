/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2013  <<user@hostname.org>>
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

/**
 * SECTION:element-scream
 *
 * FIXME:Describe scream here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! scream ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gst/gst.h>
#include <stdlib.h>

#include "gstscream.h"

GST_DEBUG_CATEGORY_STATIC(gst_scream_debug);
#define GST_CAT_DEFAULT gst_scream_debug

/* Filter signals and args */
enum {
  /* FILL ME */
  LAST_SIGNAL,
  PING_NOTIFY
};

static guint signals[LAST_SIGNAL] = {0};

enum { PROP_0, PROP_SILENT, PROP_PING, PROP_TX, PROP_RX };

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE(
    "sink", GST_PAD_SINK, GST_PAD_ALWAYS, GST_STATIC_CAPS("ANY"));

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE(
    "src", GST_PAD_SRC, GST_PAD_ALWAYS, GST_STATIC_CAPS("ANY"));

#define gst_scream_parent_class parent_class
G_DEFINE_TYPE(GstSCReAM, gst_scream, GST_TYPE_ELEMENT);

static void gst_scream_set_property(GObject *object, guint prop_id,
                                    const GValue *value, GParamSpec *pspec);
static void gst_scream_get_property(GObject *object, guint prop_id,
                                    GValue *value, GParamSpec *pspec);

static gboolean gst_scream_sink_event(GstPad *pad, GstObject *parent,
                                      GstEvent *event);
static GstFlowReturn gst_scream_chain(GstPad *pad, GstObject *parent,
                                      GstBuffer *buf);

static void ping_changed_cb();

/* GObject vmethod implementations */

/* initialize the scream's class */
static void gst_scream_class_init(GstSCReAMClass *klass) {

  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *)klass;
  gstelement_class = (GstElementClass *)klass;

  gobject_class->set_property = gst_scream_set_property;
  gobject_class->get_property = gst_scream_get_property;

  g_object_class_install_property(
      gobject_class, PROP_SILENT,
      g_param_spec_boolean("silent", "Silent", "Produce verbose output ?",
                           FALSE, G_PARAM_READWRITE));

  g_object_class_install_property(
      gobject_class, PROP_PING,
      g_param_spec_boolean("ping", "Ping", "Ping  function  ", FALSE,
                           G_PARAM_READWRITE));

  g_object_class_install_property(
      gobject_class, PROP_TX,
      g_param_spec_boolean("tx", "Tx", "Transmitter unit", FALSE,
                           G_PARAM_READWRITE));

  g_object_class_install_property(
      gobject_class, PROP_RX,
      g_param_spec_boolean("rx", "Rx", "Receiver unit    ", FALSE,
                           G_PARAM_READWRITE));

  gst_element_class_set_details_simple(
      gstelement_class, "SCReAM", "GStream Klass, scream alteration",
      "Scream element", "Group of LTU Students. See Github for info: "
                        "https://github.com/gSCReAM/gscream");

  gst_element_class_add_pad_template(gstelement_class,
                                     gst_static_pad_template_get(&src_factory));
  gst_element_class_add_pad_template(
      gstelement_class, gst_static_pad_template_get(&sink_factory));
}

static void ping_changed_cb() { // g_print("+");
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static void gst_scream_init(GstSCReAM *screamObj) {
  screamObj->sinkpad = gst_pad_new_from_static_template(&sink_factory, "sink");

  gst_pad_set_event_function(screamObj->sinkpad,
                             GST_DEBUG_FUNCPTR(gst_scream_sink_event));
  gst_pad_set_chain_function(screamObj->sinkpad,
                             GST_DEBUG_FUNCPTR(gst_scream_chain));
  GST_PAD_SET_PROXY_CAPS(screamObj->sinkpad);
  gst_element_add_pad(GST_ELEMENT(screamObj), screamObj->sinkpad);

  screamObj->srcpad = gst_pad_new_from_static_template(&src_factory, "src");
  GST_PAD_SET_PROXY_CAPS(screamObj->srcpad);
  gst_element_add_pad(GST_ELEMENT(screamObj), screamObj->srcpad);

  screamObj->silent = FALSE;
  screamObj->ping = FALSE;
  screamObj->tx = FALSE;
  screamObj->rx = FALSE;

  g_signal_connect(screamObj, "notify", G_CALLBACK(ping_changed_cb), NULL);
}

static void gst_scream_set_property(GObject *object, guint prop_id,
                                    const GValue *value, GParamSpec *pspec) {
  GstSCReAM *screamObj = GST_SCREAM(object);

  switch (prop_id) {
  case PROP_SILENT:
    screamObj->silent = g_value_get_boolean(value);
    break;

  case PROP_TX:
    screamObj->tx = g_value_get_boolean(value);
    break;

  case PROP_PING:
    screamObj->ping = g_value_get_boolean(value);
    g_object_notify(G_OBJECT(screamObj), "ping");
    break;

  case PROP_RX:
    screamObj->rx = g_value_get_boolean(value);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

static void gst_scream_get_property(GObject *object, guint prop_id,
                                    GValue *value, GParamSpec *pspec) {
  GstSCReAM *screamObj = GST_SCREAM(object);

  switch (prop_id) {

  case PROP_SILENT:
    g_value_set_boolean(value, screamObj->silent);
    break;

  case PROP_PING:
    g_value_set_boolean(value, screamObj->ping);
    break;

  case PROP_TX:
    g_value_set_boolean(value, screamObj->tx);
    break;

  case PROP_RX:
    g_value_set_boolean(value, screamObj->rx);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

/* GstElement vmethod implementations */

/* this function handles sink events */
static gboolean gst_scream_sink_event(GstPad *pad, GstObject *parent,
                                      GstEvent *event) {
  gboolean ret;
  GstSCReAM *screamObj;

  screamObj = GST_SCREAM(parent);

  switch (GST_EVENT_TYPE(event)) {
  case GST_EVENT_CAPS: {
    GstCaps *caps;

    gst_event_parse_caps(event, &caps);
    /* do something with the caps */

    /* and forward */
    ret = gst_pad_event_default(pad, parent, event);
    break;
  }

  case GST_EVENT_QOS: {
    GstQOSType type;
    gdouble proportion;
    GstClockTimeDiff diff;
    GstClockTime timestamp;

    gst_event_parse_qos(event, &type, &proportion, &diff, &timestamp);

    ret = gst_pad_push_event(screamObj->sinkpad, event);
    break;
  }
  default:
    ret = gst_pad_event_default(pad, parent, event);
    break;
  }
  return ret;
}

/* chain function
 * this function does the actual processing
 */

static GstFlowReturn gst_scream_chain(GstPad *pad, GstObject *parent,
                                      GstBuffer *buf) {
  GstSCReAM *screamObj;
  screamObj = GST_SCREAM(parent);

  if (screamObj->ping == TRUE) {
  } else if (screamObj->ping == FALSE) {
  }

  if (screamObj->tx == TRUE) {
    // TODO: Do transmitter stuff
    // Implement Scream algorithm here, check rpt queue and send appropriate
    // amount of packets
    GValue a = G_VALUE_INIT;
    g_value_init(&a, G_TYPE_BOOLEAN);
    g_value_set_boolean(&a, 1);

    gst_scream_set_property(G_OBJECT(screamObj), PROP_PING, &a,
                            G_PARAM_READWRITE);
  }

  if (screamObj->rx == TRUE) {
    // TODO: Do receiver stuff
    // Receive packets, notify tx of packets );
    // gst_scream_get_property(G_OBJECT(screamObj), PROP_PING
    //                                    G_PARAM_READWRITE);
  }

  if (screamObj->rx == TRUE && screamObj->ping == TRUE) {
    g_print("Wohoo! \n");
  }

  /* just push out the incoming buffer without touching it */
  return gst_pad_push(screamObj->srcpad, buf);
}

/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean scream_init(GstPlugin *scream) {
  /* debug category for fltering log messages
   *
   * exchange the string 'Template scream' with your description
   */

  GST_DEBUG_CATEGORY_INIT(gst_scream_debug, "scream", 0, "Congestion control");

  return gst_element_register(scream, "scream", GST_RANK_NONE,
                              GST_TYPE_MYFILTER);
}

/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "myfirstscream"
#endif

/* gstreamer looks for this structure to register screams
 *
 * exchange the string 'Template scream' with your scream description
 */
GST_PLUGIN_DEFINE(GST_VERSION_MAJOR, GST_VERSION_MINOR, scream,
                  "Congestion control using the SCReAM algorithm. Developed by "
                  "LTU students for Ericsson.",
                  scream_init, VERSION, "LGPL", "GStreamer",
                  "http://gstreamer.net/")
