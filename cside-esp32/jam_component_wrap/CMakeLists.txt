idf_component_register(SRCS "jam_component_wrap.c" "../../jamout.c"
                    INCLUDE_DIRS "../include"
                    REQUIRES esp_netif esp_event)
