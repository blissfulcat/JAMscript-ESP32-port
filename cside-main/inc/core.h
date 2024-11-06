#ifndef CORE_H
#define CORE_H

/* STRUCTS & TYPEDEFS */
typedef struct _corestate_t
{
    char *device_id;
    int serial_num;    
    // int default_port;
} corestate_t;

/* FUNCTION PROTOTYPES */
corestate_t *core_init( int serialnum);
void core_destroy(corestate_t *cs);
void core_setup(corestate_t *cs);

#endif