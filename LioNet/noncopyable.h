/**
 * @file noncopyable.h
 * @brief 不可拷贝对象封装
 */
#ifndef __LIONET_NONCOPYABLE_H__
#define __LIONET_NONCOPYABLE_H__

namespace LioNet {

/**
 * @brief 对象无法拷贝，赋值
 */
class Noncopyable {
 public:
  Noncopyable() = default;

  ~Noncopyable() = default;

  Noncopyable(const Noncopyable&) = delete;

  Noncopyable& operator=(const Noncopyable&) = delete;
};
}  // namespace LioNet

#endif