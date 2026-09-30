idf_component_register(
    SRCS
        "main.c"

        "usb/usbHost.c"
        "usb/usbStorage.c"
        "usb/usbPlayer.c"
        "usb/usbState.c"
        "usb/usbLibrary.c"
        "usb/usbDecoder.c"

        "media/mediaOutput.c"

        "cdc/cdc_protocol.c"
        "cdc/cdc_state.c"
        "cdc/cdc_uart.c"     

        "wifi/wifiHttp.c"
        "wifi/wifiService.c"

        "manager/playerManager.c"

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
        "manager"
        "components/pub"
        "components/real"
)