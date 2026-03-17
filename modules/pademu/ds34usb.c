#include <tamtypes.h>
#include <usb/usbd.h>
#include <thbase.h>
#include <stdio.h>
#include "pademu.h"

// مصفوفة الربط (Mapping Table)
static const struct {
    u8 key;
    u16 btn;
} key_map[] = {
    {0x1A, PAD_UP}, {0x16, PAD_DOWN}, {0x04, PAD_LEFT}, {0x07, PAD_RIGHT},
    {0x52, PAD_UP}, {0x51, PAD_DOWN}, {0x50, PAD_LEFT}, {0x4F, PAD_RIGHT},
    {0x2C, PAD_CROSS}, {0x1D, PAD_CIRCLE}, {0x1B, PAD_SQUARE}, {0x06, PAD_TRIANGLE},
    {0x28, PAD_START}, {0x29, PAD_SELECT}, {0x14, PAD_L1}, {0x08, PAD_R1}
};

static u8 kbd_data[8] __attribute__((aligned(64)));
static int kbd_endpoint = -1;

void kbd_callback(int resultCode, int bytes, void *arg) {
    if (resultCode == USB_RC_OK) {
        u16 new_btns = 0xFFFF;
        for (int i = 2; i < 8; i++) {
            if (kbd_data[i] == 0) continue;
            for (int j = 0; j < (sizeof(key_map)/sizeof(key_map[0])); j++) {
                if (kbd_data[i] == key_map[j].key) {
                    new_btns &= ~key_map[j].btn;
                }
            }
        }
        pademu_set_buttons(0, new_btns);
        UsbInterruptTransfer(kbd_endpoint, kbd_data, 8, kbd_callback, NULL);
    }
}

int kbd_probe(int devId) {
    UsbDeviceDescriptor *dev = UsbGetDeviceStaticDescriptor(devId);
    if (dev && (dev->bDeviceClass == USB_CLASS_HID || dev->bDeviceClass == 0)) return 1;
    return 0;
}

int kbd_connect(int devId) {
    UsbConfigurationDescriptor *conf = UsbGetConfigurationDescriptor(devId, 0);
    if (!conf) return -1;
    
    // محرك بسيط للوصول لنقطة النهاية (Endpoint)
    kbd_endpoint = UsbOpenEndpoint(devId, NULL); 
    UsbInterruptTransfer(kbd_endpoint, kbd_data, 8, kbd_callback, NULL);
    return 0;
}

int kbd_disconnect(int devId) {
    kbd_endpoint = -1;
    return 0;
}

UsbDriver kbd_driver = { NULL, NULL, "kbd_emu", kbd_probe, kbd_connect, kbd_disconnect };

int ds34usb_init(u8 pads, u8 options) {
    UsbRegisterDriver(&kbd_driver);
    return 1;
}

// إضافة كل الدوال المتوقعة لمنع أخطاء الـ Linker
void ds34usb_reset() {}
int ds34usb_get_status(int port) { return 1; }
void ds34usb_set_rumble(u8 port, u8 low, u8 high) {}
void ds34usb_set_led(u8 port, u8 led) {}
void ds34usb_set_bdaddr(u8 port, u8 *bdaddr) {}
