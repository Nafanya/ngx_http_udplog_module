#include <nginx.h>
#include <ngx_config.h>
#include <ngx_core.h>

/* Structs */

typedef struct {
    // TODO: udp log target address
} ngx_http_udplog_main_conf_t;

static ngx_stream_module_t ngx_stream_udplog_module_ctx {
    NULL,                                  /* preconfiguration */
    ngx_stream_udplog_init,                /* postconfiguration */

    ngx_stream_udplog_create_main_conf,    /* create main conf */
    NULL,                                  /* init main conf */

    NULL,                                  /* create server conf */
    NULL                                   /* merge server conf */
};

ngx_module_t ngx_stream_udplog_module = {
    NGX_MODULE_V1,
    &ngx_stream_udplog_module_ctx,         /* module context */
    ngx_stream_udplog_commands,            /* module directives */
    NGX_STREAM_MODULE,                     /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};

static ngx_int_t
ngx_stream_udplog_init(ngx_conf_t *cf) {

    return NGX_OK;
}

statinc void *
ngx_stream_udplog_create_main_conf(ngx_conf_t *cf) {
    //    ngx_stream_udplog_main_conf_t *conf;
    //
    //    conf = ngx_pcalloc(cf->pool, sizeof(ngx_stream_udplog_main_conf_t));
    //    if (conf == NULL) {
    //        return NULL;
    //    }
    //
    // TODO: init here
    //
    //    return conf;
}


/* One top-level command to init udplog */
static ngx_command_t ngx_http_udplog_commands[] = {

    { ngx_string("udp_log"),               /* directive */
      NGX_STREAM_MAIN_CONF|NGX_CONF_TAKE2, /* main ctx, 2 args (host, port) */
      ngx_stream_udplog_set_options,       /* config setup function */
      NGX_STREAM_MAIN_CONF_OFFSET,         /* write cmd args to main config */
      0,                                   /* offset in struct TODO */
      NULL },                              /* ptr to postprocess directive */

    ngx_null_command
};

static char *
ngx_stream_udplog_set_options(ngx_conf_t *cfm ngx_command_t *cmd, void *conf) {
    
}
