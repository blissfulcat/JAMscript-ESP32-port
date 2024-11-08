set(ZENOH_SRC_DIR "~/zenoh-pico/src")
set(ZENOH_INC_DIR "~/zenoh-pico/include")
idf_component_register(
        SRC_DIRS "${ZENOH_SRC_DIR}/api"
        SRC_DIRS "${ZENOH_SRC_DIR}/collections"
        SRC_DIRS "${ZENOH_SRC_DIR}/link"
        SRC_DIRS "${ZENOH_SRC_DIR}/link/config"
        SRC_DIRS "${ZENOH_SRC_DIR}/link/multicast"
        SRC_DIRS "${ZENOH_SRC_DIR}/link/unicast"
        SRC_DIRS "${ZENOH_SRC_DIR}/net"
        SRC_DIRS "${ZENOH_SRC_DIR}/protocol"
        SRC_DIRS "${ZENOH_SRC_DIR}/protocol/codec"
        SRC_DIRS "${ZENOH_SRC_DIR}/protocol/definitions"
        SRC_DIRS "${ZENOH_SRC_DIR}/session"
        SRC_DIRS "${ZENOH_SRC_DIR}/transport"
        SRC_DIRS "${ZENOH_SRC_DIR}/transport/common"
        SRC_DIRS "${ZENOH_SRC_DIR}/transport/multicast"
        SRC_DIRS "${ZENOH_SRC_DIR}/transport/raweth"
        SRC_DIRS "${ZENOH_SRC_DIR}/system"
        SRC_DIRS "${ZENOH_SRC_DIR}/system/espidf"
        #SRC_DIRS "${ZENOH_SRC_DIR}/system/freertos_plus_tcp"
        SRC_DIRS "${ZENOH_SRC_DIR}/transport/unicast"
        SRC_DIRS "${ZENOH_SRC_DIR}/utils"
        INCLUDE_DIRS "${ZENOH_INC_DIR}"
        REQUIRES esp_driver_uart
)
