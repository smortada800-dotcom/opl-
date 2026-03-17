#include <tamtypes.h>
#include <usb/usbd.h>
#include <thbase.h>
#include "pademu.h"

// مصفوفة الأزرار الأساسية
static const struct { u8 k; u16 b; } m[] = {
    {0x1A, 0x0010}, {0x16, 0x0040}, {0x04, 0x0080}, {0x07, 0x0020}, // WASD -> Up, Down, Left, Right
    {0x2C, 0x4000}, {0x28, 0x0008}, {0x29, 0x0001}, {0x1D, 0x2000}  // Space, Enter, Esc, Z
};

static u8 d[8] __attribute__((aligned(64)));
static int ep = -1;

void cb(int r, int b, void *a) {
    if (r == 0) {
        u16 bt = 0xFFFF;
        for (int i = 2; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                if (d[i] == m[j].k) bt &= ~m[j].b;
            }
        }
        pademu_set_buttons(0, bt);
        UsbInterruptTransfer(ep, d, 8, cb, NULL);
    }
}

int pr(int id) { return 1; }
int cn(int id) {
    ep = UsbOpenEndpoint(id, NULL);
    UsbInterruptTransfer(ep, d, 8, cb, NULL);
    return 0;
}

int dc(int id) { ep = -1; return 0; }

UsbDriver dr = { NULL, NULL, "kbd", pr, cn, dc };

int ds34usb_init(u8 p, u8 o) {
    UsbRegisterDriver(&dr);
    return 1;
}

// دوال فارغة لمنع أخطاء الربط (Linker Errors)
void ds34usb_reset() {}
int ds34usb_get_status(int p) { return 1; }
void ds34usb_set_rumble(u8 p, u8 l, u8 h) {}
void ds34usb_set_led(u8 p, u8 l) {}
void ds34usb_set_bdaddr(u8 p, u8 *b) {}
