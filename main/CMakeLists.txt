idf_component_register(
    SRCS
        "main.c"

        "usb/usbHost.c"
        "usb/usbStorage.c"
        
        "media/mediaLibrary.c"
        "media/mediaPlayer.c"
        "media/mediaDecoder.c"
        "media/mediaOutput.c"

        "cdc/cdc_protocol.c"
        "cdc/cdc_state.c"
        "cdc/cdc_uart.c"     
        "cdc/cdc.c"

        "wifi/wifiCore.c"

        "components/mp3dec.c"
        "components/mp3tabs.c"

        "components/real/bitstream.c"
        "components/real/buffers.c"
        "components/real/dct32.c"
        "components/real/dequant.c"
        "components/real/dqchan.c"
        "components/real/huffman.c"
        "components/real/hufftabs.c"
        "components/real/imdct.c"
        "components/real/polyphase.c"
        "components/real/scalfact.c"
        "components/real/stproc.c"
        "components/real/subband.c"
        "components/real/trigtabs.c"               
    INCLUDE_DIRS
        "."
        "usb"
        "media"
        "cdc"
        "wifi"
        "components/pub"
        "components/real"
)