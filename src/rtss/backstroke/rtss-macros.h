// headers
#define AVPUSH_RESTORE_DISPOSE_HEADER(type,typename) \
 private:\
  std::deque<std::pair<type*, type> > data_container_##typename;  \
  inline void restore_##typename();\
  inline void dispose_##typename();\
 public:\
  inline type* avpush(type* address); \

// implementations
#define AVPUSH_RESTORE_DISPOSE_IMPLEMENTATION(type,mytypename,typeenum) \
inline void Backstroke::RunTimeStateStore::restore_##mytypename() {\
  std::pair<type*,type> p=data_container_##mytypename.back();     \
  data_container_##mytypename.pop_back();\
  restore_assignment_inside_macro(p);\
}\
inline void Backstroke::RunTimeStateStore::dispose_##mytypename() {\
  data_container_##mytypename.pop_front();\
}\
type* Backstroke::RunTimeStateStore::avpush(type* address) { \
    if(!is_stack_ptr(address)) {\
      currentEventRecord->stack_bitype.push(typeenum);\
      data_container_##mytypename.push_back(std::make_pair(address,*address)); \
    }\
  return address;\
}\
\


#define CASE_ENUM_RESTORE(enumname,mytypename) \
  case BITYPE_##enumname: restore_##mytypename();break

#define CASE_ENUM_DISPOSE(enumname,mytypename) \
  case BITYPE_##enumname: dispose_##mytypename();break

#define CASE_ENUM_SIZEOF(enumname,mytypename) \
  case BITYPE_##enumname: return sizeof(mytypename)

