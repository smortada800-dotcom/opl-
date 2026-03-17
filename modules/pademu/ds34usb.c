#include <tamtypes.h>
#include <usb/usbd.h>
#include <thbase.h>
#include <stdio.h>
#include "pademu.h"

// مصفوفة لتعيين مفاتيح الكيبورد (HID Scancodes) إلى أزرار PS2
static const struct {
    u8 key;
    u16 btn;
} key_map[] = {
    {0x1A, PAD_UP},      // W -> فوق
    {0x16, PAD_DOWN},    // S -> تحت
    {0x04, PAD_LEFT},    // A -> يسار
    {0x07, PAD_RIGHT},   // D -> يمين
    {0x52, PAD_UP},      // Arrow Up
    {0x51, PAD_DOWN},    // Arrow Down
    {0x50, PAD_LEFT},    // Arrow Left
    {0x4F, PAD_RIGHT},   // Arrow Right
    {0x2C, PAD_CROSS},   // Space -> X
    {0x1D, PAD_CIRCLE},  // Z -> O
    {0x1B, PAD_SQUARE},  // X -> Square
    {0x06, PAD_TRIANGLE},// C -> Triangle
    {0x28, PAD_START},   // Enter -> Start
    {0x29, PAD_SELECT},  // Escape -> Select
    {0x14, PAD_L1},      // Q -> L1
    {0x08, PAD_R1}       // E -> R1
};

static u8 kbd_data[8];
static int kbd_endpoint = -1;

// دالة معالجة البيانات القادمة من الكيبورد
void kbd_callback(int resultCode, int bytes, void *arg) {
    if (resultCode == USB_RC_OK) {
        u16 new_btns = 0xFFFF; // الحالة الافتراضية (لا ضغط)

        // فحص المفاتيح المضغوطة (تبدأ من البايت الثالث في تقرير HID)
        for (int i = 2; i < 8; i++) {
            if (kbd_data[i] == 0) continue;
            for (int j = 0; j < (sizeof(key_map)/sizeof(key_map[0])); j++) {
                if (kbd_data[i] == key_map[j].key) {
                    new_btns &= ~key_map[j].btn;
                }
            }
        }
        // إرسال الحالة إلى محرك PADEMU في OPL
        pademu_set_buttons(0, new_btns);
        
        // طلب البيانات التالية (Loop)
        UsbInterruptTransfer(kbd_endpoint, kbd_data, 8, kbd_callback, NULL);
    }
}

// دالة التعرف على الكيبورد عند توصيله
int kbd_probe(int devId) {
    UsbDeviceDescriptor *dev = UsbGetDeviceStaticDescriptor(devId);
    if (dev->bDeviceClass == USB_CLASS_HID || dev->bDeviceClass == 0) return 1;
    return 0;
}

int kbd_connect(int devId) {
    UsbConfigurationDescriptor *conf = UsbGetConfigurationDescriptor(devId, 0);
    UsbInterfaceDescriptor *iface = &conf->Interface[0];
    UsbEndpointDescriptor *endp = &iface->Endpoint[0];

    kbd_endpoint = UsbOpenEndpoint(devId, endp);
    UsbInterruptTransfer(kbd_endpoint, kbd_data, 8, kbd_callback, NULL);
    return 0;
}

int kbd_disconnect(int devId) {
    kbd_endpoint = -1;
    return 0;
}

// تعريف الدريفر للنظام
UsbDriver kbd_driver = { NULL, NULL, "kbd_emu", kbd_probe, kbd_connect, kbd_disconnect };

int ds34usb_init(u8 pads, u8 options) {
    UsbRegisterDriver(&kbd_driver);
    return 1;
}

// دوال فارغة للحفاظ على التوافق مع ملفات OPL الأخرى
void ds34usb_reset() {}
int ds34usb_get_status(int port) { return 1; }
