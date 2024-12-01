#include "zenoh.h"

/* Connection parameters for Zenoh */
#define MODE "peer"
#define CONNECT "udp/224.0.0.224:7446#iface=eth0"

/* Scouting parameters */
#define PRINT_HELLO true// Print out scout results on zenoh_scout() callback

/* PRIVATE FUNCTIONS */
static void fprintzid(FILE *stream, z_id_t zid) {
    unsigned int zidlen = _z_id_len(zid);
    if (zidlen == 0) {
        fprintf(stream, "None");
    } else {
        fprintf(stream, "Some(");
        for (unsigned int i = 0; i < zidlen; i++) {
            fprintf(stream, "%02X", (int)zid.id[i]);
        }
        fprintf(stream, ")");
    }
}

static void fprintwhatami(FILE *stream, z_whatami_t whatami) {
    z_view_string_t s;
    z_whatami_to_view_string(whatami, &s);
    fprintf(stream, "\"%.*s\"", (int)z_string_len(z_loan(s)), z_string_data(z_loan(s)));
}

static void fprintlocators(FILE *stream, const z_loaned_string_array_t *locs) {
    fprintf(stream, "[");
    for (unsigned int i = 0; i < z_string_array_len(locs); i++) {
        fprintf(stream, "\"");
        const z_loaned_string_t *str = z_string_array_get(locs, i);
        fprintf(stream, "%.*s", (int)z_string_len(str), z_string_data(str));
        fprintf(stream, "\"");
        if (i < z_string_array_len(locs) - 1) {
            fprintf(stream, ", ");
        }
    }
    fprintf(stream, "]");
}

static void fprinthello(FILE *stream, const z_loaned_hello_t *hello) {
    fprintf(stream, "Hello { zid: ");
    fprintzid(stream, z_hello_zid(hello));
    fprintf(stream, ", whatami: ");
    fprintwhatami(stream, z_hello_whatami(hello));
    fprintf(stream, ", locators: ");
    fprintlocators(stream, zp_hello_locators(hello));
    fprintf(stream, " }");
}

static void callback(z_loaned_hello_t *hello, void *context) {
#ifdef PRINT_HELLO
    fprinthello(stdout, hello);
    fprintf(stdout, "\n");
#endif
    (*(int *)context)++;
}

static void drop(void *context) {
    int count = *(int *)context;
    free((int*)context);
    if (!count) {
        printf("Did not find any zenoh process.\n");
    } else {
        printf("Dropping scout results.\n");
    }
}

/* PUBLIC FUNCTIONS */
zenoh_t* zenoh_init() {
    /* Initialize Zenoh Session and other parameters */
    zenoh_t* zenoh = calloc(1, sizeof(zenoh_t));
    z_owned_config_t config;
    z_config_default(&config);
    zp_config_insert(z_loan_mut(config), Z_CONFIG_MODE_KEY, MODE);
    if (strcmp(CONNECT, "") != 0) {
        zp_config_insert(z_loan_mut(config), Z_CONFIG_CONNECT_KEY, CONNECT);
    }

    /* Open Zenoh session */
    int retval = z_open(&zenoh->z_session, z_move(config), NULL); 
    if (retval < 0) {
        printf("Unable to open Zenoh session! Error code: %d\n", retval);
        return NULL;
    }
    return zenoh;
}

void zenoh_destroy(zenoh_t* zenoh) {
    /* FREE ALL MEMBERS THAT ARE ALLOCATED USING MALLOC, CALLOC */
    if (zenoh == NULL) {
        return;
    }
    z_drop(z_move(zenoh->z_pub));
    z_drop(z_move(zenoh->z_sub));
    z_drop(z_move(zenoh->z_session));
    free(zenoh);
}

/*
* TODO: How do we determine which of the nodes are J nodes? 
* Should we send a message (pub) and wait for a response for identification ?
* TODO: Refactor: we do not need the zenoh object as a parameter
*/
bool zenoh_scout() {
    int *context = (int *)malloc(sizeof(int));
    *context = 0;
    z_owned_config_t config;
    z_config_default(&config);
    z_owned_closure_hello_t closure;
    z_closure_hello(&closure, callback, drop, context);
    zp_config_insert(z_loan_mut(config), Z_CONFIG_MODE_KEY, MODE);
    if (strcmp(CONNECT, "") != 0) {
        zp_config_insert(z_loan_mut(config), Z_CONFIG_CONNECT_KEY, CONNECT);
    }
    z_scout(z_config_move(&config), z_closure_hello_move(&closure), NULL);
    return true;
}

bool zenoh_declare_sub(zenoh_t* zenoh, const char* key_expression, zenoh_callback_t* callback, void* cb_arg) {
    /* Make sure we don't accidentally dereference a null pointer ... */
    if (zenoh == NULL) {
        return false;
    }
    z_owned_closure_sample_t cb;
    z_closure(&cb, callback); 
    cb._val.context = cb_arg; /* Pass in as an argument to the callback */
    z_view_keyexpr_t ke;
    z_view_keyexpr_from_str_unchecked(&ke, key_expression);
    if (z_declare_subscriber(z_loan(zenoh->z_session), &zenoh->z_sub , z_loan(ke), z_move(cb), NULL) < 0) {
        return false;
    }
    return true;
}

bool zenoh_declare_pub(zenoh_t* zenoh, const char* key_expression) {
    /* Make sure we don't accidentally dereference a null pointer ... */
    if (zenoh == NULL) {
        return false;
    }

    z_view_keyexpr_t ke;
    z_view_keyexpr_from_str_unchecked(&ke, key_expression);
    if (z_declare_publisher(z_loan(zenoh->z_session), &zenoh->z_pub , z_loan(ke), NULL) < 0) {
        return false;
    }
    return true;
}


void zenoh_start_read_task(zenoh_t* zenoh) {
    /* Make sure we don't accidentally dereference a null pointer ... */
    if (zenoh == NULL) {
        return;
    }
    zp_start_read_task(z_loan_mut(zenoh->z_session), NULL);
} 
 

void zenoh_start_lease_task(zenoh_t* zenoh) {
    /* Make sure we don't accidentally dereference a null pointer ... */
    if (zenoh == NULL) {
        return;
    }
    zp_start_lease_task(z_loan_mut(zenoh->z_session), NULL);
} 
 
bool zenoh_publish(zenoh_t* zenoh, const char* message) {
    /* Make sure we don't accidentally dereference a null pointer ... */
    if (zenoh == NULL) {
        return false;
    }

    z_owned_bytes_t payload;
    z_bytes_copy_from_str(&payload, message);
    if (z_publisher_put(z_loan(zenoh->z_pub), z_move(payload), NULL) != Z_OK) {
        return false;
    }
    return true;
}
