#ifndef __TRACEX_EVENT_H__
#define __TRACEX_EVENT_H__

#include <stdint.h>

enum TRACEX_event_id_t
{

};

enum TRACEX_event_info1
{

};
enum TRACEX_event_info2
{

};
enum TRACEX_event_info3
{

};
enum TRACEX_event_info4
{

};

struct TRACEX_event_t
{
    uint32_t     thread_pointer;
    uint32_t     thread_priority;
    enum TRACEX_event_id_t event_id;
    uint32_t     time_stamp;
    enum TRACEX_event_info1 info1;
    enum TRACEX_event_info2 info2;
    enum TRACEX_event_info3 info3;
    enum TRACEX_event_info4 info4;
};

#endif