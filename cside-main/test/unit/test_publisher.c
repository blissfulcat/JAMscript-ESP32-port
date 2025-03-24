#include "cnode.h"
#include "zenoh.h"
#include "test_macros.h"
#include "utils.h"

int test_publisher(void) {
    cnode_t* cnode = cnode_init(0, NULL);
    TEST_ASSERT(cnode != NULL);
    cnode->node_id = "uuid4_2";
    TEST_ASSERT(cnode_start(cnode));
    TEST_ASSERT(zenoh_publish(cnode->zenoh, "Test Message from board 1", cnode->zenoh_pub_request));
    cnode_destroy(cnode);
    return TEST_SUCCESS;
}

int run_publisher(void) {
    return test_publisher();
}
