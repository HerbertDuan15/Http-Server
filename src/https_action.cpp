//bug 刷新首页可绕过登陆
#include "https_server.h"
#include <openssl/crypto.h>
#include "my_config.h"
#include <unistd.h>
#define BLOCK_SIZE 1024

extern CONF_INFO config;

std::map<std::string, std::string> 
content_type {
    { "txt", "text/plain" },
        { "c", "text/plain" },
        { "cpp", "text/plain" },
        { "h", "text/plain" },
        { "html", "text/html" },
        { "htm", "text/htm" },
        { "css", "text/css" },
        { "gif", "image/gif" },
        { "jpg", "image/jpeg" },
        { "jpeg", "image/jpeg" },
        { "png", "image/png" },
        { "ico", "image/x-icon"},
        { "pdf", "application/pdf" },
        { "ps", "application/postsript" }};

int php_htoi(char *s)  
{  
    int value;  
    int c;  
  
    c = ((unsigned char *)s)[0];  
    if (isupper(c))  
        c = tolower(c);  
    value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;  
  
    c = ((unsigned char *)s)[1];  
    if (isupper(c))  
        c = tolower(c);  
    value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;  
  
    return (value);  
}  
/* }}} */  
  
/* {{{ URL解码，提取自PHP 5.2.17 
   用法：string urldecode(string str_source) 
   时间：2012-8-14 By Dewei 
*/  
std::string urldecode(std::string &str_source)  
{  
    char const *in_str = str_source.c_str();  
    int in_str_len = strlen(in_str);  
    int out_str_len = 0;  
    std::string out_str;  
    char *str;  
  
    str = strdup(in_str);  
    char *dest = str;  
    char *data = str;  
  
    while (in_str_len--) {  
        if (*data == '+') {  
            *dest = ' ';  
        }  
        else if (*data == '%' && in_str_len >= 2 && isxdigit((int) *(data + 1))   
            && isxdigit((int) *(data + 2))) {  
                *dest = (char) php_htoi(data + 1);  
                data += 2;  
                in_str_len -= 2;  
        } else {  
            *dest = *data;  
        }  
        data++;  
        dest++;  
    }  
    *dest = '\0';  
    out_str_len =  dest - str;  
    out_str = str;  
    free(str);  
    return out_str;  
}  
/* }}} */  

void
login_action(struct evhttp_request *req){
    if(auth(req) == 1)
        return send_page(req,
                (config.root + config.source_root + config.default_page).c_str());
    else
        return send_page(req, 
                (config.root + config.source_root + config.index_page).c_str());
}

void
file_action(struct evhttp_request *req){
    if (evhttp_request_get_command (req) == EVHTTP_REQ_POST)
        return file_upload(req);
    else if(evhttp_request_get_command (req) == EVHTTP_REQ_GET)
        return file_download(req);
    else
        return send_page(req, 502);    
}

void
media_action(struct evhttp_request *req){
}

int 
auth(struct evhttp_request *req){
    evkeyvalq* headers = evhttp_request_get_input_headers(req);
    // struct evkeyval *header;
    // for (header = headers->tqh_first; header; header = header->next.tqe_next) {
    //     if(std::string(header->key) == "Cookie"){
    //         uuid = header->value;
    //         break;
    //     }
    // }
    // struct evkeyvalq * kv = evhttp_request_get_input_headers(req);
    // printf("%s\n", evhttp_find_header(headers, "SetCookie"));
    const char* cookie = evhttp_find_header(headers, "Cookie");
    if(cookie == nullptr || (strstr(cookie, "UUID=") == nullptr)){
        if (evhttp_request_get_command (req) == EVHTTP_REQ_POST){
            struct evbuffer *buf = evhttp_request_get_input_buffer (req);
            evbuffer_add(buf, "", -1);    /* NUL-terminate the buffer */
            char *payload = (char *) evbuffer_pullup (buf, -1);
            // int post_data_len = evbuffer_get_length(buf);
            // char request_data_buf[4096] = {0};
            // memcpy(request_data_buf, payload, post_data_len);
            std::vector<std::string> sub_paths;
            boost::split(sub_paths, payload, boost::is_any_of( "&=" ), boost::token_compress_on);
            if(sub_paths[1].empty() || sub_paths[3].empty())
                return 0;
            else if(sub_paths[1] == sub_paths[3]){//设置cookie
                boost::uuids::uuid a_uuid = boost::uuids::random_generator()(); 
                std::string uuid_string = boost::uuids::to_string(a_uuid);
                std::string cookie = "UUID=" + uuid_string + ";Path=/";
                // struct evbuffer *buf = evbuffer_new();
                // if (buf == NULL) 
                //     return 0;
                evhttp_add_header(evhttp_request_get_output_headers(req), "Set-Cookie", cookie.c_str());
                std::string head_tail = "text/html; charset=UTF-8";
                return 1;
            }else
                return 0;
        }else{
            // send_page(req, 000);//不支持该方法
            return 0;
        }
    }
    return 1;
}

void action_cdn(struct evhttp_request *req){
    struct evbuffer *evb = NULL;
    std::string URI(evhttp_request_get_uri (req));
    std::vector<std::string> sub_paths;
    boost::split(sub_paths, URI, boost::is_any_of( ",/." ), boost::token_compress_on);
    if(sub_paths[2] == "login")
        return login_action(req);
    else if(sub_paths[2] == "file")
        return file_action(req);
    else if(sub_paths[2] == "media")
        return media_action(req);
    else
        return send_page(req, 502);
}

char *
load_file(const char * filename){
    // std::cout << "希望读取文件" << filename << std::endl;
    FILE * file = fopen(filename,"r");
    if(file == nullptr)
        return nullptr;
    struct stat statbuf;
    stat(filename,&statbuf);
    char * html = (char *)malloc(statbuf.st_size);
    if(html == nullptr)
        return nullptr;
    memset(html, 0 , statbuf.st_size);
    fread(html, sizeof(char), statbuf.st_size, file);
    return html;
}

void 
file_download(struct evhttp_request *req){
    struct evbuffer *buf = evhttp_request_get_input_buffer (req);
    // evbuffer_add(buf, "", -1);    /* NUL-terminate the buffer */
    // std::string head_info((char *)evbuffer_pullup (buf, -1));
    // std::string filename = "skulshady_gcam_5.1.018_Mod_v5.1_manual.apk";
    std::string filename = "2019-07-23-165628.webm";
    std::string path = config.root + config.data_root + filename;
    std::cout << "下载读出:" << path << std::endl;
    // evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", "text/plain");
    evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", "video/webm");
    // evhttp_add_header(evhttp_request_get_output_headers(req), "Connection", "Keep-Alive");
    // evhttp_add_header(evhttp_request_get_output_headers(req), "Transfer-Encoding", "chunked");

    struct conn_ctx *cctx = (struct conn_ctx *) malloc(sizeof(struct conn_ctx));
    cctx->req = req;
    cctx->conn = evhttp_request_get_connection(req);
    // std::fstream file(path);
    cctx->file = fopen(path.c_str(), "r");
    if(cctx->file == nullptr){
        std::cout << "打开文件失败" << std::endl;
    }
    evhttp_send_reply_start(req, 200, "OK");
    return reply_chunk_transfer(cctx->conn, cctx);
}

    void 
reply_chunk_transfer(struct evhttp_connection *conn, void *ctx)
{
    struct conn_ctx *cctx = (struct conn_ctx *)ctx;
    if(feof(cctx->file) != 0){
        evhttp_send_reply_end(cctx->req);
        fclose(cctx->file);
        free(cctx);
        return;
    }else{
        struct evbuffer *buf = evbuffer_new();
        char *dummy = (char *) malloc(BLOCK_SIZE * sizeof(char));  // dynamically create the data in real cases.
        memset(dummy, 0, BLOCK_SIZE * sizeof(char));
        fread(dummy, sizeof(char), BLOCK_SIZE, cctx->file);
        // std::cout << "chunk" << std::endl
        //     << dummy << std::endl;
        std::stringstream data;
        try {
            copy(Base64EncodeIterator(dummy), 
                    Base64EncodeIterator(dummy+BLOCK_SIZE),
                    std::ostream_iterator<char>(data));
        } catch ( ... ) {
            std::cout << "base64 加密错误" << std::endl;
            return;
        }
        std::string encode = data.str();
        evbuffer_add(buf, encode.c_str(), encode.length());
        // evbuffer_add(buf, dummy, strlen(dummy));
        // std::cout << "传输数据块" << std::endl << dummy << "结束块" << std::endl;
        std::cout << "base64加密数据块" << std::endl << encode << std::endl;
        evhttp_send_reply_chunk_with_cb(cctx->req, buf, reply_chunk_transfer, cctx);
        evbuffer_free(buf);
        free(dummy);
    }
}

void 
file_upload(struct evhttp_request *req){
    struct evbuffer *buf = evhttp_request_get_input_buffer(req);
    int post_data_len = evbuffer_get_length(buf);
    std::cout << "上传数据大小" << post_data_len << std::endl;
    char * data = (char *) malloc(post_data_len * sizeof(char));
    evbuffer_remove(buf, data, post_data_len);
    std::string urlcode(data);
    std::string base64code = urldecode(urlcode);
    // std::cout << "上传内容" << base64code << std::endl;
    // std::cout << "数据内容" << t_dat << std::endl;
    std::vector<std::string> datas;
    boost::split(datas, base64code, boost::is_any_of( "&" ), boost::token_compress_on);
//     boost::regex filename_expression("filename=");   
//     boost::regex content_expression("(base64*)");   
//     boost::cmatch filename_match;
//     if(boost::regex_match(datas[0].c_str(), filename_match, filename_expression))//regex_match:匹配算法，用于测试一个字符串是否和正则式匹配   
//     {  
//         for(int i=0;i<filename_match.size();i++)  
//                     std::cout<<"file"<<filename_match[i].str()<< std::endl;  
//     }
//     boost::cmatch content_match;  
//     if(boost::regex_match(datas[1].c_str(), content_match, content_expression))//regex_match:匹配算法，用于测试一个字符串是否和正则式匹配   
//     {  
//         for(int i=0;i<content_match.size();i++)  
//                     std::cout<<"content"<<content_match[i].str()<< std::endl;  
//     }
    std::string filename(datas[0].begin()+strlen("filename="), datas[0].end());
    std::string path = config.root + config.data_root + filename;
    std::string content_match(datas[1].begin()+datas[1].find("base64,")+7, datas[1].end());
    std::stringstream content;
    try {
        copy(Base64DecodeIterator(content_match.begin()), 
                Base64DecodeIterator(content_match.end()), 
                std::ostream_iterator<char>(content));
    }catch (...){
    //        try {
    //     copy(Base64DecodeIterator(content_match[0].begin()), 
    //             Base64DecodeIterator(content_match[0].end()), 
    //             std::ostream_iterator<char>(content));
    // }catch (...){
        std::cout << "base64解密错误" << std::endl;
        return;
    }
    // std::cout << std::endl << "解密后数据" << content.str() << std::endl;
    // std::string path = config.root + config.data_root + "test";
    std::ofstream file(path);
    std::cout << "上传写入" << path << std::endl;
    if (file.is_open()){
        file << content.str();
        file.close();
    }
    free(data);
    struct evbuffer *rbuf = evbuffer_new();
    evbuffer_add_printf(rbuf, "received file %s", filename.c_str());
    evhttp_send_reply(req, HTTP_OK, "SUCCESS", rbuf);
}

void
action_login(struct evhttp_request *req){
    if(evhttp_request_get_command (req) == EVHTTP_REQ_POST){
        struct evbuffer *evb = NULL;
        const char *uri = evhttp_request_get_uri (req);
        struct evhttp_uri *decoded = NULL;
        char * file_name = NULL;
        // int http_state = check_valid(uri, &file_name);
        // struct evbuffer *buf = evbuffer_new();
        struct evbuffer *buf = evhttp_request_get_input_buffer (req);
        std::stringstream data;
        data << (char *) evbuffer_pullup (buf, -1);
        std::string line;
        struct evbuffer *rbuf = evbuffer_new();
        evbuffer_add_printf(rbuf, "%s", "OK");
        evhttp_send_reply(req, HTTP_OK, "NULL", rbuf);
    }else{
        send_page(req, 502);
    }
}

void file_cdn(struct evhttp_request *req){
    std::string URI(evhttp_request_get_uri (req));
    std::vector<std::string> sub_paths;
    boost::split(sub_paths, URI, boost::is_any_of( ",/." ), boost::token_compress_on);
    size_t start = URI.find(".", URI.find(".")+1);
    std::string sub_path = URI.substr(start+1);
    //     std::cout << "file_cdn(%s)" << URI << std::endl;
    // for(auto s : sub_paths)
    //     std::cout << "sub " << s << " | ";
    // std::cout << std::endl;
    if(URI.size() <= 1)
        return send_page(req, (config.root + config.default_page).c_str());
    else{
        std::string url = config.root + config.source_root;
        if(sub_paths[2] == "login")
            url += (config.login + sub_path);
        else if(sub_paths[2] == "file")
            url += (config.file + sub_path);
        else if(sub_paths[2] == "states")
            url += (config.states + sub_path);
        else
            url += (config.test + sub_path);
        if(-1 == access(url.c_str(), 0))
            return send_page(req, 404);
        // else if(sub_paths[1] != config.source_root)
        //     return send_page(req, 403);
        else{
            // std::cout  << "url: " << url << std::endl;
            return send_page(req, url);
        }
    }
}

void
default_cb(struct evhttp_request *req, void *arg){
    std::cout << "请求地址: " << evhttp_request_uri(req) << std::endl;
    // evhttp_send_error(req, 404, "404");
    // struct evkeyvalq * kv = evhttp_request_get_input_headers(req);
    // printf("%s\n", evhttp_find_header(kv, "SetCookie"));
    // struct evbuffer *evb = NULL;
    std::string URI(evhttp_request_get_uri (req));
    // int valid = auth(req);
    if(auth(req) == 1){
        // return send_page(req, config.root+config.source_root+config.default_page);
        // else{
        std::cout << "请求地址  " << URI.size() << std::endl;
        if(URI.size() <= 1)
            return send_page(req, (config.root + config.source_root + config.default_page));
        else{
            std::vector<std::string> sub_paths;
            boost::split(sub_paths, URI, boost::is_any_of( ",/." ), boost::token_compress_on);
            for(auto it : sub_paths)
                std::cout << "1  " << it << std::endl;
            if(sub_paths[1] == "action")
                return action_cdn(req);
            else if(sub_paths[1] == "file")
                return file_cdn(req);
            else
                return send_page(req, 403);
        }
    }else
        return send_page(req, config.root+config.source_root+config.index_page);
    }

    void 
        send_page(struct evhttp_request *req, std::string page){
            struct evbuffer *buf = evbuffer_new();
            if (buf == NULL) 
                return;
            // std::cout << "send_page::page=" << page << std::endl; 
            std::vector<std::string> sub_paths;
            boost::split(sub_paths, page, boost::is_any_of( ",/." ), boost::token_compress_on);
            std::string head_tail = "text/" + sub_paths.back() + "; charset=UTF-8";
            evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", head_tail.c_str());
            evhttp_add_header(evhttp_request_get_output_headers(req), "Connection", "Keep-Alive");
            // std::cout << "尝试读取文件" << page.c_str() << std::endl;
            // std::cout << "当前路径" << getcwd(nullptr,0) << std::endl;    
            // char * file = load_file(page.c_str());
            char * file = load_file(page.c_str());
            evbuffer_add_printf(buf, "%s", file);
            // std::cout << "发送数据: \n" << buf << std::endl;
            // printf("发送数据: \n %s",buf);
            free(file);
            evhttp_send_reply(req, HTTP_OK, "OK", buf);
        }

    void send_page(struct evhttp_request *req, int states){
        struct evbuffer *buf = evbuffer_new();
        if (buf == NULL) return;
        evhttp_send_error(req, states, std::to_string(states).c_str());
    }
