/*
 * ws protocol handler plugin for "lws-minimal"
 *
 * Written in 2010-2019 by Andy Green <andy@warmcat.com>
 *
 * This file is made available under the Creative Commons CC0 1.0
 * Universal Public Domain Dedication.
 *
 * This version holds a single message at a time, which may be lost if a new
 * message comes.  See the minimal-ws-server-ring sample for the same thing
 * but using an lws_ring ringbuffer to hold up to 8 messages at a time.
 */

#if !defined(LWS_PLUGIN_STATIC)
#define LWS_DLL
#define LWS_INTERNAL
#include <libwebsockets.h>
#endif

#include <string.h>

/* one of these created for each message */

struct msg {
  void *payload; /* is malloc'd */
  size_t len;
};

/* one of these is created for each client connecting to us */

struct per_session_data__minimal {
  struct per_session_data__minimal *pss_list;
  struct lws *wsi;
  int last; /* the last message number we sent */
};

/* one of these is created for each vhost our protocol is used with */

struct per_vhost_data__minimal {
  struct lws_context *context;
  struct lws_vhost *vhost;
  const struct lws_protocols *protocol;

  struct per_session_data__minimal *pss_list; /* linked-list of live pss*/

  struct msg amsg; /* the one pending message... */
  int current;     /* the current message number we are caching */
};

/* destroys the message when everyone has had a copy of it */
#define LWS_PLUGIN_PROTOCOL_MINIMAL                                            \
  {                                                                            \
    "lws-minimal", callback_minimal, sizeof(struct per_session_data__minimal), \
        128, 0, NULL, 0                                                        \
  }
