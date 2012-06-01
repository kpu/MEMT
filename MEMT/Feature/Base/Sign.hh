#ifndef _MEMT_Feature_Base_Sign_h
#define _MEMT_Feature_Base_Sign_h

namespace feature {
namespace base {

// Optimizer sign constraints.  These are not enforced by MEMT but passed to the client if requested.  That way the client doesn't have to know about positional features.  
typedef enum {ANY_WEIGHT=0, POSITIVE_WEIGHT=1, NEGATIVE_WEIGHT=-1} WeightSign;

} // namespace base
} // namesapce feature

#endif // _MEMT_Feature_Base_Sign_H
