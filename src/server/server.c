#include "iex/iex.h"
#include <server/server.h>

#define LWS_PLUGIN_STATIC
#include "protocol_lws_minimal.c"

int SERVER_INTERRUPTED = 0;

static int callback_minimal(struct lws *wsi, enum lws_callback_reasons reason,
			void *user, void *in, size_t len);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

static struct lws_protocols protocols[] = {
	{ "http", lws_callback_http_dummy, 0, 0 },
	LWS_PLUGIN_PROTOCOL_MINIMAL,
	{ NULL, NULL, 0, 0 } /* terminator */
};

static const struct lws_http_mount mount = {
	/* .mount_next */		NULL,		/* linked-list "next" */
	/* .mountpoint */		"/",		/* mountpoint URL */
	/* .origin */			"./web",  /* serve from dir */
	/* .def */			"index.html",	/* default filename */
	/* .protocol */			NULL,
	/* .cgienv */			NULL,
	/* .extra_mimetypes */		NULL,
	/* .interpret */		NULL,
	/* .cgi_timeout */		0,
	/* .cache_max_age */		0,
	/* .auth_mask */		0,
	/* .cache_reusable */		0,
	/* .cache_revalidate */		0,
	/* .cache_intermediaries */	0,
	/* .origin_protocol */		LWSMPRO_FILE,	/* files in a dir */
	/* .mountpoint_len */		1,		/* char count */
	/* .basic_auth_login_file */	NULL,
};
#pragma GCC diagnostic push

void sigint_handler(int sig) {
  (void) sig;
  SERVER_INTERRUPTED = 1;
  IEX_SIGNAL_INTER = 1;
  iex_stop_parse();
}

static int callback_minimal(struct lws *wsi, enum lws_callback_reasons reason,
			void *user, void *in, size_t len) {
	struct per_session_data__minimal *pss =
			(struct per_session_data__minimal *)user;
	struct per_vhost_data__minimal *vhd =
			(struct per_vhost_data__minimal *)
			lws_protocol_vh_priv_get(lws_get_vhost(wsi),
					lws_get_protocol(wsi));
	int m;

	switch (reason) {
	case LWS_CALLBACK_PROTOCOL_INIT:
		vhd = lws_protocol_vh_priv_zalloc(lws_get_vhost(wsi),
				lws_get_protocol(wsi),
				sizeof(struct per_vhost_data__minimal));
		vhd->context = lws_get_context(wsi);
		vhd->protocol = lws_get_protocol(wsi);
		vhd->vhost = lws_get_vhost(wsi);
		break;

	case LWS_CALLBACK_ESTABLISHED:
    /* add ourselves to the list of live pss held in the vhd */
		lws_ll_fwd_insert(pss, pss_list, vhd->pss_list);
		pss->wsi = wsi;
		pss->last = vhd->current;
		break;

	case LWS_CALLBACK_CLOSED:
		/* remove our closing pss from the list of live pss */
		lws_ll_fwd_remove(struct per_session_data__minimal, pss_list,
				  pss, vhd->pss_list);
		break;

	case LWS_CALLBACK_SERVER_WRITEABLE:

		if (!vhd->amsg.payload)
			break;

		if (pss->last == vhd->current)
			break;

		/* notice we allowed for LWS_PRE in the payload already */

    /*
		m = lws_write(wsi, ((unsigned char *)vhd->amsg.payload) +
			      LWS_PRE, vhd->amsg.len, LWS_WRITE_TEXT);
		if (m < (int)vhd->amsg.len) {
			lwsl_err("ERROR %d writing to ws\n", m);
			return -1;
		}
		pss->last = vhd->current;
    */
		break;

	case LWS_CALLBACK_RECEIVE:
		if (vhd->amsg.payload) {
      struct msg* msg = &vhd->amsg;
      free(msg->payload);
      msg->payload = NULL;
      msg->len = 0;
    }

    char* response = parse_message(in, len);
    if (!response)
      break;
    size_t response_len = strlen(response);

		vhd->amsg.len = response_len;
		/* notice we over-allocate by LWS_PRE */
    vhd->amsg.payload = malloc(LWS_PRE + response_len);
		if (!vhd->amsg.payload) {
			lwsl_user("OOM: dropping\n");
			break;
		}
    
		memcpy((char *)vhd->amsg.payload + LWS_PRE, response, response_len);
		vhd->current++;

    free(response);

    m = lws_write(wsi, ((unsigned char *)vhd->amsg.payload) +
			      LWS_PRE, vhd->amsg.len, LWS_WRITE_TEXT);
		if (m < (int)vhd->amsg.len) {
			lwsl_err("ERROR %d writing to ws\n", m);
			return -1;
		}
		pss->last = vhd->current;
    free(vhd->amsg.payload);
    vhd->amsg.payload = NULL;
    vhd->amsg.len = 0;
		break;

	default:
		break;
	}

	return 0;
}


void* server_start(void* s) {
  (void) s;
  
  struct lws_context_creation_info info;
	struct lws_context *context;
	int n = 0, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE
			/* for LLL_ verbosity above NOTICE to be built into lws,
			 * lws must have been configured and built with
			 * -DCMAKE_BUILD_TYPE=DEBUG instead of =RELEASE */
			/* | LLL_INFO */ /* | LLL_PARSER */ /* | LLL_HEADER */
			/* | LLL_EXT */ /* | LLL_CLIENT */ /* | LLL_LATENCY */
			/* | LLL_DEBUG */;

	signal(SIGINT, sigint_handler);


  lws_set_log_level(logs, NULL);
  lwsl_user("LWS minimal ws server | visit http://localhost:7681 (-s = use TLS / https)\n");

  memset(&info, 0, sizeof info); /* otherwise uninitialized garbage */
  info.port = 7681;
  info.mounts = &mount;
  info.protocols = protocols;
  info.vhost_name = "localhost";
  info.ws_ping_pong_interval = 20;
  info.options =
    LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;
	
  context = lws_create_context(&info);
  if (!context) {
	  lwsl_err("lws init failed\n");
    exit(1);
	  return NULL;
  }

  while (n >= 0 && !SERVER_INTERRUPTED)
	  n = lws_service(context, 0);

  lws_context_destroy(context);
  return 0;
}
