idf_component_register(
    SRCS
        "main.c"

        "usb/usbHost.c"
        "usb/usbStorage.c"
        "usb/usbLibrary.c"
        "usb/usbPlayer.c"
        "usb/usbDecoder.c"
        
        "media/mediaOutput.c"

        "cdc/cdc_protocol.c"
        "cdc/cdc_state.c"
        "cdc/cdc_uart.c"     
        "cdc/cdc.c"

        "bluetooth/bm83.c"
        "bluetooth/bm83_uart.c"
        "bluetooth/bm83_protocol.c"
        "bluetooth/bm83_state.c"
        "bluetooth/bm83_player.c"

        "wifi/wifiService.c"
        "wifi/wifiHttp.c"

        "sourceManager/sourceManager.c"

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
        "bluetooth"
        "wifi"
        "sourceManager"
        "components/pub"
        "components/real"

    EMBED_TXTFILES
        "wifi/web/index.html"
)