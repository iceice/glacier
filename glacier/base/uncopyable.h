#ifndef GLACIER_BASE_UNCOPYABLE_
#define GLACIER_BASE_UNCOPYABLE_

//
// Private copy constructor and copy assignment ensure classes
// derived from class Uncopyable cannot be copied.
// 
class Uncopyable {
 protected:
  Uncopyable() {}
  ~Uncopyable() {}

 private:
  Uncopyable(const Uncopyable &);
  Uncopyable &operator=(const Uncopyable &);
};

#endif  // GLACIER_BASE_UNCOPYABLE_
