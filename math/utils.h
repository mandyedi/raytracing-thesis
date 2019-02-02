#ifndef UTILS_H
#define UTILS_H

template <typename T>
inline T abs( const T &t )
{
    return t >= 0 ? t : -t;
}

class Utils
{
public:
    Utils();
    ~Utils();

    static inline bool IsNull( float d );
};

/*static*/ inline bool Utils::IsNull( float f )
{
  return abs( f ) <= 0.00001f;
}

#endif // UTILS_H
