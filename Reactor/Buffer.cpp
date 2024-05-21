#include "Buffer.h"

Buffer::Buffer(uint16_t sep)
:sep_(sep)
{

}

Buffer::~Buffer()
{

}

// 把数据追加到buf_中
void Buffer::append(const char *data, size_t size)
{
    buf_.append(data, size);
}

//返回buf_的大小
size_t Buffer::size()
{
    return buf_.size();
}

//返回buf_的首地址
const char *Buffer::data()  //返回的指针所指向的数据是不可修改的
{
    return buf_.data();  //data()函数返回一个指向string对象中存储的字符数据的指针
}

//清空buf_
void Buffer::clear()
{
    buf_.clear();
}

void Buffer::erase(size_t pos, size_t nn)
{
    buf_.erase(pos, nn);
}

// 报文长度（头部） + 报文内容
void Buffer::appendwithsep(const char *data, size_t size)
{
    if(sep_ == 0)
        buf_.append(data, size);   // 添加报文头部信息（报文的长度）
    else if(sep_ == 1)
    {
        buf_.append((char*)&size, 4); //处理报文长度（头部）
        buf_.append(data, size); // 处理报文内容
    }
    else
    {
        /* ... */
    }
}

// 从buf_中拆分出一个报文，存放在ss中，如果buf_没有报文，返回false
bool Buffer::pickmessage(std::string &ss)
{
    if(buf_.size() == 0) return false;

    if(sep_ == 0)  // 没有分隔符
    {
        ss = buf_;
        buf_.clear();
    }
    else if(sep_ == 1) // 四字节的报头
    {
        //用指定报文长度的方式解决TCP粘包分包问题
        int len;
        memcpy(&len, buf_.data(), 4);  // 从接收缓冲区获取报文头部

        //如果接收缓冲区中的数据量小于报文头部，说明buf_中的报文内容不完整
        if(buf_.size() < len + 4) return false;

        //如果报文内容是完整的，取出来
        ss = buf_.substr(4, len);      // 从buf_中获取一段报文 
        buf_.erase(0, len+4);          // 删除已获取的报文
    }

    return true;
}