#ifndef BASE_UNCOPYABLE_
#define BASE_UNCOPYABLE_

/*
  Private copy constructor and copy assignment ensure classes 
  derived from class Uncopyable cannot be copied.
*/
class Uncopyable
{
protected:
    Uncopyable() {}
    ~Uncopyable() {}

private:
    Uncopyable(const Uncopyable &);
    Uncopyable &operator=(const Uncopyable &);
};

#endif // BASE_UNCOPYABLE_
