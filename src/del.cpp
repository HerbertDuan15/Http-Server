
// void
// login_cb (struct evhttp_request *req, void *arg){
//     std::cout << "进入login——cb回调" << std::endl;
//     struct evbuffer *evb = NULL;
//     struct evhttp_uri *decoded = NULL;
    // if (evhttp_request_get_command (req) == EVHTTP_REQ_GET){
    //     // printf("GET\n");
    //     struct evbuffer *buf = evbuffer_new();
    //     if (buf == NULL) return;
    //     evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", "text/html; charset=UTF-8");
    //     evhttp_add_header(evhttp_request_get_output_headers(req), "Connection", "Keep-Alive");
    //     char * file = load_file("./html/login.html");
    //     evbuffer_add_printf(buf, "%s", file);
    //     free(file);
    //     evhttp_send_reply(req, HTTP_OK, "OK", buf);
    //     return;
//    }else if (evhttp_request_get_command (req) == EVHTTP_REQ_POST){ 
//    if (evhttp_request_get_command (req) == EVHTTP_REQ_POST){ 
        /* Decode the payload */
        // struct evbuffer *buf = evhttp_request_get_input_buffer (req);
        // evbuffer_add (buf, "", 1);    /* NUL-terminate the buffer */
        // char *payload = (char *) evbuffer_pullup (buf, -1);
        // int post_data_len = evbuffer_get_length(buf);
        // char request_data_buf[4096] = {0};
        // memcpy(request_data_buf, payload, post_data_len);
        // printf("[post_data][%d]=\n %s\n", post_data_len, payload);

        // evhttp_add_header(evhttp_request_get_output_headers(req), "Server", MYHTTPD_SIGNATURE);
        // evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", "text/html; charset=UTF-8");
        // evhttp_add_header(evhttp_request_get_output_headers(req), "Connection", "Keep-Alive");

        // evb = evbuffer_new ();
        // if(do_auth(payload) == 0){
        //     char * file = load_file("./html/login.html");
        //     evbuffer_add_printf(evb, "%s", file);    //将封装好的evbuffer 发送给客户端
        //     free(file);
        // }else{
        //     char * file = load_file("./html/UploadServer-master/html/index.html");
        //     evbuffer_add_printf(evb, "%s", file);    //将封装好的evbuffer 发送给客户端
        //     free(file);    
        // }
        // evhttp_send_reply(req, HTTP_OK, "OK", evb);
        // if (evb)
        //     evbuffer_free (evb);
        // // printf("[response]:\n");

    // }else
    //     return send_page(req, 502);
//}


// void
// state_cb(struct evhttp_request *req, void *arg, int state){
//     if(state = 404){
//         evhttp_add_header(evhttp_request_get_output_headers(req), "Server", MYHTTPD_SIGNATURE);
//         evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", "text/html; charset=UTF-8");
//         evhttp_add_header(evhttp_request_get_output_headers(req), "Connection", "close");
//             char * file = load_file("./html/login.html");
//             struct evbuffer * evb = evbuffer_new ();
//             evbuffer_add_printf(evb, "%s", file);    //将封装好的evbuffer 发送给客户端
//             free(file);
//         evhttp_send_reply (req, 404, "NOT FOUND", NULL);
//         return;
//     }else{
//         return;
//     }
// }


#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/http.h>

struct event_base *evbase_ctx;
struct evhttp *evhttp_ctx;

struct trickle_ctx {
	struct evhttp_request *req;
	struct event *tm_ev;
	int remaining;
};

void trickle_cb(int fd, short events, void *ctx)
{
	struct trickle_ctx *tinfo = ctx;

	struct evbuffer *buf = evbuffer_new();
	evbuffer_add_printf(buf, "just a bit of data ...\n");
	evhttp_send_reply_chunk(tinfo->req, buf);
	evbuffer_free(buf);

	tinfo->remaining--;
	if (tinfo->remaining <= 0) {
		event_free(tinfo->tm_ev);
		evhttp_send_reply_end(tinfo->req);
		free(tinfo);
		return;
	}
	struct timeval timeout = { 1, 0 };
	event_add(tinfo->tm_ev, &timeout);
}

void request_cb(struct evhttp_request *req, void *ctx)
{
	evhttp_send_reply_start(req, 402, "OK");

	struct trickle_ctx *tinfo = malloc(sizeof(struct trickle_ctx));
	tinfo->req = req;
	tinfo->tm_ev = event_new(evbase_ctx, -1, 0, trickle_cb, tinfo);
	tinfo->remaining = 10;
	struct timeval timeout = { 1, 0 };
	event_add(tinfo->tm_ev, &timeout);
}

int main()
{
	evbase_ctx = event_base_new();
	evhttp_ctx = evhttp_new(evbase_ctx);

	signal(SIGPIPE, SIG_IGN);

	evhttp_set_gencb(evhttp_ctx, request_cb, NULL);
	if (evhttp_bind_socket(evhttp_ctx, "0.0.0.0", 2345) < 0) {
		perror("failed to bind port 2345");
		exit(1);
	}

	printf("listening on port 2345\n");
	event_base_loop(evbase_ctx, 0);
	// CTRL-C will send SIGINT and terminate the process
	return 0;
}



#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/http.h>

static const int SIZE = 4000000;

struct conn_ctx {
        struct evhttp_connection *conn;
        struct evhttp_request *req;
        int remaining;
};

void reply_chunk_cb(struct evhttp_connection *conn, void *ctx)
{
        struct conn_ctx *cctx = ctx;
        struct evbuffer *buf = evbuffer_new();
        uint8_t *dummy = (uint8_t *) malloc(SIZE);  // dynamically create the data in real cases.
        memset(dummy, 3, SIZE);
	evbuffer_add(buf, dummy, SIZE);
        evhttp_send_reply_chunk_with_cb(cctx->req, buf, reply_chunk_cb, cctx);
        evbuffer_free(buf);
        free(dummy);

	cctx->remaining--;
	if (cctx->remaining <= 0) {
                evhttp_send_reply_end(cctx->req);
                free(cctx);
                return;
        }   
}

void request_cb(struct evhttp_request *req, void *ctx)
{
	evhttp_send_reply_start(req, 200, "OK");

        struct conn_ctx *cctx = malloc(sizeof(struct conn_ctx));
        cctx->req = req;
        cctx->conn = evhttp_request_get_connection(req);
        cctx->remaining = 50;
        reply_chunk_cb(cctx->conn, cctx);
}

int
main(int argc, char **argv)
{
        struct evhttp *http;
        struct evhttp_bound_socket *handle;

        ev_uint16_t port = 480;

        if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
                return (1);

        base = event_base_new();
        if (!base) {
                return 1;
        }

        /* Create a new evhttp object to handle requests. */
        http = evhttp_new(base);
        if (!http) {
                return 1;
        }

        evhttp_set_cb(http, request_cb, NULL);

        /* Now we tell the evhttp what port to listen on */
        handle = evhttp_bind_socket_with_handle(http, "0.0.0.0", port);
        if (!handle) {
                return 1;
        }

        {
                /* Extract and display the address we're listening on. */
                struct sockaddr_storage ss;
                evutil_socket_t fd;
                ev_socklen_t socklen = sizeof(ss);
                char addrbuf[128];
                void *inaddr;
                const char *addr;
                int got_port = -1;
                fd = evhttp_bound_socket_get_fd(handle);
                memset(&ss, 0, sizeof(ss));
                if (getsockname(fd, (struct sockaddr *)&ss, &socklen)) {
                        return 1;
                }
                if (ss.ss_family == AF_INET) {
                        got_port = ntohs(((struct sockaddr_in*)&ss)->sin_port);
                        inaddr = &((struct sockaddr_in*)&ss)->sin_addr;
                } else if (ss.ss_family == AF_INET6) {
                        got_port = ntohs(((struct sockaddr_in6*)&ss)->sin6_port);
                        inaddr = &((struct sockaddr_in6*)&ss)->sin6_addr;
                } else {
                        return 1;
                }
                addr = evutil_inet_ntop(ss.ss_family, inaddr, addrbuf,
                    sizeof(addrbuf));
                if (addr) {
                        printf("Listening on %s:%d\n", addr, got_port);
                } else {
                        return 1;
                }
        }

        event_base_dispatch(base);

        return 0;
}