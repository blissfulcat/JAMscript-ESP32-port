#include "zenoh.h"
#include "system_manager.h"

static void data_handler(z_loaned_sample_t* sample, void* arg) {
    z_view_string_t keystr;
    z_keyexpr_as_view_string(z_sample_keyexpr(sample), &keystr);
    z_owned_string_t value;
    z_bytes_to_string(z_sample_payload(sample), &value);

    printf(" >> [Subscriber handler] Received ('%.*s': '%.*s')\n", (int)z_string_len(z_view_string_loan(&keystr)),
           z_string_data(z_view_string_loan(&keystr)), (int)z_string_len(z_string_loan(&value)),
           z_string_data(z_string_loan(&value)));

    z_string_drop(z_string_move(&value));
}

void app_main(void)
{
    /* Init wifi */
    system_manager_t* sm = system_manager_init();
    if (!system_manager_wifi_init(sm)) {
        printf("Could not init wifi \r\n");
        exit(-1);
    }

    /* Init Zenoh session */
    zenoh_t* zn = zenoh_init();
    if (zn == NULL) {
        printf("Could not init Zenoh session \r\n");
        exit(-1);
    }

    zenoh_start_lease_task(zn);
    zenoh_start_read_task(zn);

    /* Declare multiple publishers */
    zenoh_pub_t z_pub_reply;
    zenoh_pub_t z_pub_request;

    if (!zenoh_declare_pub(zn, "app/replies/up", &z_pub_reply)) {
        printf("Could not declare pub 1 \r\n");
        exit(-1);
    }

    if (!zenoh_declare_pub(zn, "app/requests/up", &z_pub_request)) {
        printf("Could not declare pub 2 \r\n");
        exit(-1);
    }

    printf("Successfully declared 2 publishers \r\n");

    if (!zenoh_declare_sub(zn, "app/**", data_handler, NULL)) {
        printf("Could not declare subscriber \r\n");
    }

    /* Loop forever */
    while (true) {

        if (!zenoh_publish(zn, "reply from cnode", &z_pub_reply)) {
            printf("Could not send message using z_pub_reply \r\n");
        }

        if (!zenoh_publish(zn, "request from cnode", &z_pub_request)) {
            printf("Could not send message using z_pub_request \r\n");
        }

        sleep(1);
    }
}
