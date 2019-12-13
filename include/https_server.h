#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>

#include <vector>
#include <iostream>
#include <sstream>      // std::stringstream
#include <string>
#include <iterator>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/regex.hpp>   

#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/crypto.h>

#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/http_compat.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>

#ifndef HTTP_SERVER_H_
#define HTTP_SERVER_H_

#define MYHTTPD_SIGNATURE   "MoCarHttpd v0.1"

#ifdef EVENT__HAVE_NETINET_IN_H
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#endif


// #define COMMON_HTTPS_PORT 8421

/**
 * This is the string the client tells the server in the POST request.
 */
#define COMMON_PASSCODE "R23"

/**
 * If an OpenSSL func	tion returns a return value indicating failure
 * (for non-pointery functions, generally 1 means success, and non-1
 * means failure), then usually it ought to have left one or more
 * entries in the OpenSSL "error stack", which is a bit of thread-local
 * state containing error information.
 *
 * This function is a simple way to handle OpenSSL errors (something
 * better may be needed in a real application), which prints the
 * name of the function which failed (which you must supply as an
 * argument), prints the OpenSSL error stack (which hopefully says
 * something meaningful) and exits.
 */
void die_most_horribly_from_openssl_error (const char *func);

void error_exit (const char *fmt, ...);

#define error_report printf
#define info_report printf

/**
 * Calls some OpenSSL setup functions, which both the client and
 * server need to do.
 */
void common_setup (void);

typedef union
{ 
    struct sockaddr_storage ss;
    struct sockaddr sa;
    struct sockaddr_in in;
    struct sockaddr_in6 i6;
} sock_hop;

struct conn_ctx {
    struct evhttp_connection *conn;
    struct evhttp_request *req;
    FILE *file;
};

typedef boost::archive::iterators::transform_width
        <boost::archive::iterators::binary_from_base64
        <std::string::const_iterator>, 8, 6> 
        Base64DecodeIterator;
typedef boost::archive::iterators::base64_from_binary
        <boost::archive::iterators::transform_width
        <std::string::const_iterator, 6, 8>> 
        Base64EncodeIterator;

std::string 
urldecode(std::string &str_source);

int 
php_htoi(char *s);

char *
load_file(const char * filename);

void 
file_download(struct evhttp_request *req);

void 
file_upload(struct evhttp_request *req);

void
default_cb(struct evhttp_request *req, void *arg);

int 
auth(struct evhttp_request *req);

void 
action_login(struct evhttp_request *req);

struct bufferevent* 
bevcb (struct event_base *base, void *arg);

void 
server_setup_certs (SSL_CTX *ctx);

int 
serve_some_https (void);

int 
echo_info(struct evhttp_bound_socket *handle);

void
send_page(struct evhttp_request *req, int states);

void
send_page(struct evhttp_request *req, std::string page);

void 
login_action(struct evhttp_request *req);

void 
file_action(struct evhttp_request *req);

void 
media_action(struct evhttp_request *req);

void 
reply_chunk_transfer(struct evhttp_connection *conn, void *req);

#endif
