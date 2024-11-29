idf_component_register(SRCS "src/main.cpp"
                            "src/web_server.cpp"
                    INCLUDE_DIRS "include"
                    EMBED_TXTFILES "spiffs/index.html")

spiffs_create_partition_image(storage spiffs FLASH_IN_PROJECT)
idf_build_set_property(PARTITION_TABLE_CUSTOM partition_table.csv)
idf_build_set_property(PARTITION_TABLE_CUSTOM_FILENAME partitions.csv)