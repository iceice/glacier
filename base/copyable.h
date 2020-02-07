#ifndef BASE_COPYABLE_
#define BASE_COPYABLE_

/*
  A tag class emphasises the objects are copyable.
  The empty base class optimization applies.
  Any derived class of copyable should be a value type.
*/
class Copyable
{
protected:
    Copyable() {}
    ~Copyable() {}
};


#endif // BASE_COPYABLE_
