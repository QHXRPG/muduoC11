#pragma once
#include <string>
#include <iostream>
#include <cstring>

class Buffer
{
private:
    // 用于存放数据
    std::string buf_;

    // 报文分隔符，
    // 0-无分隔符(固定长度，视频会议)； 
    // 1-四字头的报头；
    // 2-"\r\n\r\n"分隔符(http协议)
    const uint16_t sep_;


public:
    Buffer(uint16_t sep = 1);
    ~Buffer();

    // 把数据追加到buf_中
    void append(const char *data, size_t size);

    //从 pos 位置开始删除 nn 个字节
    void erase(size_t pos, size_t nn);

    //返回buf_的大小
    size_t size();

    //返回buf_的首地址
    const char *data();  //返回的指针所指向的数据是不可修改的

    //清空buf_
    void clear();

    // 报文长度（头部） + 报文内容
    void appendwithsep(const char *data, size_t size);

    // 从buf_中拆分出一个报文，存放在ss中，如果buf_没有报文，返回false
    bool pickmessage(std::string &ss);
};
