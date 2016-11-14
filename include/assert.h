#ifndef _ASSERT_H_
  #define _ASSERT_H_




#undef assert

#ifdef NDEBUG
    #define assert ((void)0)
#else
    void _Assert(char *exp, char *file, char *line);
    #define _STR(X) _VAL(X)
    #define _VAL(X) #X
    #define assert(_Expression) ((_Expression)?(void)0:_Assert(#_Expression, __FILE__, _STR(__LINE__)))
#endif











#endif  /* _ASSERT_H_ */

