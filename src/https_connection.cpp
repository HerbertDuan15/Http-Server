#include "https_server.h"
#include "my_config.h"

extern CONF_INFO config;

// unsigned short serverPort = COMMON_HTTPS_PORT;

struct bufferevent* bevcb (struct event_base *base, void *arg){ 
    struct bufferevent* r;
    SSL_CTX *ctx = (SSL_CTX *) arg;

    r = bufferevent_openssl_socket_new (base,
            -1,
            SSL_new (ctx),
            BUFFEREVENT_SSL_ACCEPTING,
            BEV_OPT_CLOSE_ON_FREE);
    return r;
}

void server_setup_certs (SSL_CTX *ctx){
    std::string certificate_chain = (config.root + config.certificate);
    std::string private_key = (config.root + config.private_key);
    // std::cout << (config.root + config.private_key).c_str() << std::endl
    //         << private_key << std::endl;
    info_report ("Loading certificate chain from '%s'\n"
            "and private key from '%s'\n",
            certificate_chain.c_str(), private_key.c_str());

    if (1 != SSL_CTX_use_certificate_chain_file (ctx, certificate_chain.c_str()))
        die_most_horribly_from_openssl_error ("SSL_CTX_use_certificate_chain_file");
    if (1 != SSL_CTX_use_PrivateKey_file (ctx, private_key.c_str(), SSL_FILETYPE_PEM))
        die_most_horribly_from_openssl_error ("SSL_CTX_use_PrivateKey_file");
    if (1 != SSL_CTX_check_private_key (ctx))
        die_most_horribly_from_openssl_error ("SSL_CTX_check_private_key");
}

/* 这个是调用openSSL提供的打印log接口 */
void die_most_horribly_from_openssl_error (const char *func){ 
    fprintf (stderr, "%s failed:\n", func);

    /* This is the OpenSSL function that prints the contents of the
     * error stack to the specified file handle. */
    ERR_print_errors_fp (stderr);

    exit (EXIT_FAILURE);
}

void set_for_https(struct evhttp * http){
    SSL_CTX *ctx = SSL_CTX_new (SSLv23_server_method ());
    SSL_CTX_set_options (ctx,
            SSL_OP_SINGLE_DH_USE |
            SSL_OP_SINGLE_ECDH_USE |
            SSL_OP_NO_SSLv2);

    EC_KEY *ecdh = EC_KEY_new_by_curve_name (NID_X9_62_prime256v1);
    if (! ecdh)
        die_most_horribly_from_openssl_error ("EC_KEY_new_by_curve_name");
    if (1 != SSL_CTX_set_tmp_ecdh (ctx, ecdh))
        die_most_horribly_from_openssl_error ("SSL_CTX_set_tmp_ecdh");

    // const char *certificate_chain = config.certificate;
    // const char *private_key = config.private_key;
    // server_setup_certs (ctx, certificate_chain, private_key);
    server_setup_certs (ctx);
    //是否可以在这里分别实现http和https通信协议
    evhttp_set_bevcb (http, bevcb, ctx);//https
}

    int 
serve_some_https (void)
{ 
    struct event_base *base;
    struct evhttp *http;
    struct evhttp_bound_socket *handle;

    if(!(base = event_base_new()) || !(http = evhttp_new(base))){ 
        fprintf (stderr, "%s fail\n",(base ? "base" : "new"));
        return 1;
    }

    set_for_https(http);
    evhttp_set_gencb (http, default_cb, NULL);

    if(! (handle = evhttp_bind_socket_with_handle (http, "0.0.0.0", config.port)))    { 
        fprintf (stderr, "couldn't bind to port %d. Exiting.\n", config.port);
        return 1;
    }

    if(1 == echo_info(handle))
        return 1;

    /* 开始阻塞监听 (永久执行) */
    event_base_dispatch (base);

    return 0;
}

int echo_info(struct evhttp_bound_socket *handle){
    sock_hop ss;
    evutil_socket_t fd;
    ev_socklen_t socklen = sizeof (ss);
    char addrbuf[128];
    void *inaddr;
    const char *addr;
    int got_port = -1;

    fd = evhttp_bound_socket_get_fd (handle);
    memset (&ss, 0, sizeof(ss));

    if (getsockname (fd, &ss.sa, &socklen)){ 
        perror ("getsockname() failed");
        return 1;
    }
    switch(ss.ss.ss_family){
        case AF_INET:
            got_port = ntohs (ss.in.sin_port);
            inaddr = &ss.in.sin_addr;
            break;
        case AF_INET6:
            got_port = ntohs (ss.i6.sin6_port);
            inaddr = &ss.i6.sin6_addr;
            break;
        default :
            fprintf (stderr, "Weird address family %d\n", ss.ss.ss_family);
            return 1;
    }

    if(addr = evutil_inet_ntop (ss.ss.ss_family, inaddr, addrbuf, sizeof (addrbuf))){
        printf ("Listening on %s:%d\n", addr, got_port);
        return 0;
    }
    else{ 
        fprintf (stderr, "evutil_inet_ntop failed\n");
        return 1;
    }
}
