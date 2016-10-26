#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <nginx.h>


typedef ngx_addr_t ngx_udplog_addr_t;


typedef struct {
    ngx_udplog_addr_t           peer_addr;
    ngx_resolver_connection_t  *udp_connection;
} ngx_udp_endpoint_t;


typedef struct {               /* MAIN conf */
    ngx_udp_endpoint_t         *endpoint;
} ngx_http_udplog_main_conf_t;


ngx_int_t         ngx_udp_connect(ngx_resolver_connection_t *uc);

static ngx_int_t  ngx_http_udplog_init(ngx_conf_t *cf);
static void       ngx_http_udplog_cleanup(void *data);
static void *     ngx_http_udplog_create_main_conf(ngx_conf_t *cf);
static char *     ngx_http_udplog_set_log(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t  ngx_http_udplog_send(ngx_udp_endpoint_t *l, u_char *buf, size_t len);


static ngx_command_t ngx_http_udplog_commands[] = {

    { ngx_string("udplog"),
      NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE2,
      ngx_http_udplog_set_log,
      NGX_HTTP_MAIN_CONF_OFFSET,
      0,
      NULL },

      ngx_null_command
};


static ngx_http_module_t ngx_http_udplog_module_ctx = {
    NULL,                                  /* preconfiguration */
    ngx_http_udplog_init,                  /* postconfiguration */

    ngx_http_udplog_create_main_conf,      /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    NULL,                                  /* create location configration */
    NULL                                   /* merge location configration */
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


ngx_int_t
ngx_http_udplog_handler(ngx_http_request_t *r) {
    u_char                      *line, *p;
    uint32_t                     crc;
    size_t                       len;
    ngx_http_udplog_main_conf_t *umcf;

    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "udplog http handler");

    len = (r->uri).len + 10; // 10 bytes for hex-formatted 32bit int, like 0xAABBCCDD
    line = ngx_pnalloc(r->pool, len);

    if (line == NULL) {
        return NGX_ERROR;
    }

    umcf = ngx_http_get_module_main_conf(r, ngx_http_udplog_module);
    crc = ngx_crc32_long((r->uri).data, (r->uri).len);
    p = ngx_sprintf(line, "%V 0x%08Xd%N", &(r->uri), crc);

    ngx_http_udplog_send(umcf->endpoint, line, p - line);

    return NGX_OK;
}


static ngx_int_t
ngx_udplog_init_endpoint(ngx_conf_t *cf, ngx_udp_endpoint_t *endpoint) {
    ngx_pool_cleanup_t         *cln;
    ngx_resolver_connection_t  *uc;

    cln = ngx_pool_cleanup_add(cf->pool, 0);
    if (cln == NULL) {
        return NGX_ERROR;
    }

    cln->handler = ngx_http_udplog_cleanup;
    cln->data = endpoint;

    uc = ngx_calloc(sizeof(ngx_resolver_connection_t), cf->log);
    if (uc == NULL) {
        return NGX_ERROR;
    }

    endpoint->udp_connection = uc;

    uc->sockaddr = endpoint->peer_addr.sockaddr;
    uc->socklen = endpoint->peer_addr.socklen;
    uc->server = endpoint->peer_addr.name;
    uc->log = cf->cycle->new_log;
    return NGX_OK;
}


static void
ngx_http_udplog_cleanup(void *data) {
    ngx_udp_endpoint_t  *e = data;

    ngx_log_debug0(NGX_LOG_DEBUG_CORE, ngx_cycle->log, 0, "udplog cleanup");

    if (e->udp_connection) {
        if (e->udp_connection->udp) {
            ngx_close_connection(e->udp_connection->udp);
        }

        ngx_free(e->udp_connection);
    }
}


static void
ngx_http_udplogger_dummy_handler(ngx_event_t *ev) {}


static ngx_int_t
ngx_http_udplog_send(ngx_udp_endpoint_t *l, u_char *buf, size_t len) {
    ssize_t                     n;
    ngx_resolver_connection_t  *uc;

    uc = l->udp_connection;

    if (uc->udp == NULL) {
        if (ngx_udp_connect(uc) != NGX_OK) {
            return NGX_ERROR;
        }
        uc->udp->data = l;
        uc->udp->read->handler = ngx_http_udplogger_dummy_handler;
        uc->udp->read->resolver = 0;
    }

    n = ngx_send(uc->udp, buf, len);

    if (n == -1) {
        return NGX_ERROR;
    }

    if ((size_t) n != (size_t) len) {
        ngx_log_error(NGX_LOG_CRIT, &(uc->log), 0, "send() incomplete");
        return NGX_ERROR;
    }

    return NGX_OK;
}


static void *
ngx_http_udplog_create_main_conf(ngx_conf_t *cf) {
    ngx_http_udplog_main_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_udplog_main_conf_t));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }

    return conf;
}


static ngx_udp_endpoint_t *
ngx_http_udplog_set_endpoint(ngx_conf_t *cf, ngx_udplog_addr_t *peer_addr) {
    ngx_http_udplog_main_conf_t    *umcf;

    umcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_udplog_module);

    if (umcf->endpoint == NULL) {
        umcf->endpoint = ngx_palloc(cf->pool, sizeof(ngx_udp_endpoint_t));
        if (umcf->endpoint == NULL) {
            return NULL;
        }
    }

    umcf->endpoint->peer_addr = *peer_addr;

    return umcf->endpoint;
}


static char *
ngx_http_udplog_set_log(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
    ngx_http_udplog_main_conf_t *umcf = conf;
    ngx_str_t                   *value;
    ngx_url_t                    u;

    value = cf->args->elts;

    umcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_udplog_module);

    ngx_memzero(&u, sizeof(ngx_url_t));

    u.host = value[1];
    u.port = ngx_atoi(value[2].data, value[2].len);

    if (ngx_inet_resolve_host(cf->pool, &u) != NGX_OK) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "%V: %s", &u.host, u.err);
        return NGX_CONF_ERROR;
    }

    umcf->endpoint = ngx_http_udplog_set_endpoint(cf, &u.addrs[0]);

    if (umcf->endpoint == NULL) {
        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_udplog_init(ngx_conf_t *cf) {
    ngx_int_t                     rc;
    ngx_http_core_main_conf_t    *cmcf;
    ngx_http_udplog_main_conf_t  *umcf;
    ngx_http_handler_pt          *h;

    umcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_udplog_module);

    if (umcf->endpoint != NULL) {
        rc = ngx_udplog_init_endpoint(cf, umcf->endpoint);

        if (rc != NGX_OK) {
            return NGX_ERROR;
        }

        cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

        h = ngx_array_push(&cmcf->phases[NGX_HTTP_LOG_PHASE].handlers);
        if (h == NULL) {
            return NGX_ERROR;
        }

        *h = ngx_http_udplog_handler;
    }

    return NGX_OK;
}
