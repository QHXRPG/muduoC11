#pragma once
#include <string>
#include <iostream>

class Buffer
{
private:
    // 用于存放数据
    std::string buf_;
public:
    Buffer(/* args */);
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
    void appendwithhead(const char *data, size_t size);
};
