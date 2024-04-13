#pragma once

class noncopyable
{
public:
    // 派生类拷贝构造会先调用基类的拷贝构造
    noncopyable(const noncopyable&) = delete;
    void operator=(const noncopyable&) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;

};
