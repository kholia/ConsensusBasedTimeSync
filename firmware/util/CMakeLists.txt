add_library(util rx_ft8.cpp rx_ft8.h)
target_link_libraries(util hardware_adc hardware_dma hardware_spi hardware_rtc pico_stdlib)
