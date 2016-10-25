#include <nginx.h>
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {
    ngx_str_t logserver_host;
    ngx_int_t logserver_port;
} ngx_http_udplog_main_conf_t;


typedef struct {
    ngx_flag_t enabled;
} ngx_http_udplog_loc_conf_t;


ngx_module_t ngx_http_udplog_module;


static ngx_int_t
ngx_http_udplog_handler(ngx_http_request_t *r) {
    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "http udplog handler");
    //r->uri;
    //ngx_http_udplog_send();
    return NGX_OK;
}


static ngx_int_t
ngx_http_udplog_init(ngx_conf_t *cf) {
    ngx_http_handler_pt *h;
    ngx_http_core_main_conf_t *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_HTTP_LOG_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    *h = ngx_http_udplog_handler;

    return NGX_OK;
}


static void *
ngx_http_udplog_create_main_conf(ngx_conf_t *cf) {
    ngx_http_udplog_main_conf_t *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_udplog_main_conf_t));
    if (conf == NULL) {
        return NULL; //TODO
    }

    conf->logserver_port = NGX_CONF_UNSET;

    return conf;
}


static void *
ngx_http_udplog_create_loc_conf(ngx_conf_t *cf) {
    ngx_http_udplog_loc_conf_t *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_udplog_loc_conf_t));
    if (conf == NULL) {
        return NULL; // TODO
    }

    conf->enabled = NGX_CONF_UNSET;

    return conf;
}


static char *
ngx_http_udplog_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child) {
    ngx_http_udplog_loc_conf_t *prev = (ngx_http_udplog_loc_conf_t*)parent;
    ngx_http_udplog_loc_conf_t *conf = (ngx_http_udplog_loc_conf_t*)child;

    ngx_conf_merge_value(conf->enabled, prev->enabled, 0);

    return NGX_CONF_OK;
}


static ngx_command_t ngx_http_udplog_commands[] = {

    { ngx_string("udplog_host"),           /* directive */
      NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE1,   /* main ctx, 2 args (host, port) */
      ngx_conf_set_str_slot,               /* config setup function, default */
      NGX_HTTP_MAIN_CONF_OFFSET,           /* write cmd args to main config */
      offsetof(ngx_http_udplog_main_conf_t, logserver_host),
      NULL },                              /* ptr to postprocess directive */

    { ngx_string("udplog_port"),
      NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      NGX_HTTP_MAIN_CONF_OFFSET,
      offsetof(ngx_http_udplog_main_conf_t, logserver_port),
      NULL },

    ngx_null_command
};


static ngx_http_module_t ngx_http_udplog_module_ctx = {
    NULL,                                  /* preconfiguration */
    ngx_http_udplog_init,                  /* postconfiguration */

    ngx_http_udplog_create_main_conf,      /* create main conf */
    NULL,                                  /* init main conf */

    NULL,                                  /* create server conf */
    NULL,                                  /* merge server conf */

    ngx_http_udplog_create_loc_conf,       /* create loc conf */
    ngx_http_udplog_merge_loc_conf         /* merge loc conf */
};


ngx_module_t ngx_http_udplog_module = {
    NGX_MODULE_V1,
    &ngx_http_udplog_module_ctx,           /* module context */
    ngx_http_udplog_commands,              /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};



