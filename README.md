# 基于libevent的https服务器

## 说明
1. 配置文件使用了json，在项目目录下的cfg文件夹下
2. source文件夹存放资源文件(html,css,js等)
3. data文件夹存放上传和下载的数据
4. crt文件夹存放了ssl证书和公钥

## 需要安装的包与工具（对于新的ubuntu来说）
    sudo apt-get install openssl
    sudo apt-get install libssl-dev
    sudo apt-get install libevent-dev
    sudo apt install gcc
    sudo apt install make
    sudo apt install make-guile
    sudo apt  install cmake
    sudo apt install g++
    apt-get install libboost-dev
    sudo apt-get install libjsoncpp-dev
    sudo apt-get install uuid-dev
    sudo apt-get install libboost-all-dev


## 目前思路
    将所有的处理分为file和action，action执行相应的功能，file处理页面，所有的请求走default_cb， 
    由该函数负责分发至file_cdn()和action_cdn()中，file_cdn和action_cdn()分发至具体的处理函数。
## 功能
### 已经实现的功能
1. https加密通信
2. 文本文档的上传（无法上传二进制文件[图片，视频]）
3. cookie的设置和免登陆（依赖UUID,没有检验过期时间）
4. 简单的GET，POST方法
### 待完善
1. chunk各种状态码的返回页面
2. 下载的动态网页搞不定

### SO
1. keep-alive选项
2. libevent有一个函数叫
    ```c
        evhttp_send_reply_start(struct evhttp_request *req)
        evhttp_send_reply_chunk(struct evhttp_request *req, struct evbuffer *databuf)
        evhttp_send_reply_end(struct evhttp_request *req)
    ```
    [参考例子1](https://gist.github.com/rgl/291085)，[参考例子2](https://stackoverflow.com/questions/20482843/serving-large-files-2gb-with-libevent-on-32-bit-system)

## 使用
    ```shell
        cd ${PROJECT_PATH}/build
        cmake ../
        make
        ./bin/server
    ```
