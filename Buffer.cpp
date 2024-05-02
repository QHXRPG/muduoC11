#include "Buffer.h"

// class Buffer
// {
// private:
//     // 用于存放数据
//     std::string buf_;
// public:
//     Buffer(/* args */);
//     ~Buffer();

//     // 把数据追加到buf_中
//     void append(const char *data, size_t size);

//     //返回buf_的大小
//     size_t size();

//     //返回buf_的首地址
//     const char *data();  //返回的指针所指向的数据是不可修改的

//     //清空buf_
//     void clear();
// };

Buffer::Buffer()
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
void Buffer::appendwithhead(const char *data, size_t size)
{
    buf_.append((char*) &size, 4);     // 添加报文头部信息（报文的长度）
    buf_.append(data, size);      // 拼接报文内容
}