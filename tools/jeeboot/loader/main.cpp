// Boot loader, using RFM69 native driver w/ JeeBoot protocol.

#define DEBUG 1     // set to 1 to include serial printf debugging. else 0

#include "sys.h"

#include "bootlogic.h"
#include "util.h"

#include "spi.h"
#include "rf69.h"
#include "flash.h"

#if DEBUG
#define D(x) x
#else
#define D(x)
#endif

RF69<SpiDev0> rf;

const int PAGE_SIZE = 64, PAGE_BASE = 0x1000;
int pageFill, pageBuf [PAGE_SIZE/sizeof(int)];

// RfDriver is used by BootLogic to talk to the RF69 driver.
class RfDriver {
public:
    int request (const void* inp, unsigned inLen, BootReply* rp) {
        // send out the request over RF
        D( printf("request # %d\n", inLen); )
        rf.send(0xC0, inp, inLen);

        // wait a limited time for a reply to come back
        int len;
        for (int i = 0; i < 100000; ++i) {
            len = rf.receive(rp, sizeof (BootReply));
            // only accept packets with special flags and coming from server
            if (len > 2 && rp->flags == 3 && rp->orig != 0 && rp->dest == 0)
                break;
        }
        rf.sleep();

        D( printf("  got # %d\n", len); )
        // D( for (int i = 0; i < len; ++i) )
        // D(     printf("%02x", ((const uint8_t*) rp)[i]); )
        // D( printf("\n"); )
        return len - 2;
    }
};

// RfDispatch is called by BootLogic when it wants to save to flash.
bool RfDispatch (int pos, const uint8_t* buf, int len) {
    D( printf("dispatch @ %d # %d\n", pos, len); )

    // last call needs to flush what remains in pageBuf
    if (buf == 0 && pageFill > 0)
        len = PAGE_SIZE - pageFill;

    while (len > 0) {
        int count = len;
        if (pageFill + count > PAGE_SIZE)
            count = PAGE_SIZE - pageFill;

        if (buf) {
            D( printf("copying %db @ %d\n", count, pageFill); )
            memcpy((uint8_t*) pageBuf + pageFill, buf, count);
            buf += count;
        } else {
            D( printf("clearing %db @ %d\n", count, pageFill); )
            memset((uint8_t*) pageBuf + pageFill, 0xFF, count);
        }

        pageFill += count;
        if (pageFill >= PAGE_SIZE) {
            int pageNum = (PAGE_BASE + pos) / PAGE_SIZE;
            D( printf("FLASH page %d pos %d\n", pageNum, pos); )
            Flash64::erase(pageNum, 1);
            Flash64::save(pageNum, pageBuf);
            pageFill = 0;

            // D( for (int i = 0; i < PAGE_SIZE; ++i) )
            // D(     printf("%02x", ((const uint8_t*) PAGE_BASE + pos)[i]); )
            // D( printf("\n"); )
        }

        pos += count;
        len -= count;
    }

    return true;
}

// the bootLogic object encapsulates the entire... boot logic
BootLogic<RfDriver,RfDispatch> bootLogic;

int main () {
    // jnp 0.4 pins assignments for the RFM69 on SPI 0
    LPC_SWM->PINASSIGN[3] = 0x11FFFFFF;
    LPC_SWM->PINASSIGN[4] = 0xFF170908;

    // set up a serial connection if debugging is enabled
    D( LPC_SWM->PINASSIGN[0] = 0xFFFFFF04; );
    D( serial.init(115200); )
    D( printf("[loader]\n"); )

    // initialise the RFM69
    rf.init(0, 42, 8686);
    //rf.encrypt("mysecret");
    rf.txPower(15); // 0 = min .. 31 = max
    D( printf("rf inited %x\n", (unsigned) pageBuf); )

    // get the 16-byte h/w id using the LPC's built-in IAP code in ROM
    uint32_t cmd = IAP_READ_UID_CMD, result[5];
    iap_entry(&cmd, result);
    D( printf("hwid %08x %08x %8x %08x\n",
                (unsigned) result[1], (unsigned) result[2],
                (unsigned) result[3], (unsigned) result[4]); )

    // let's get in touch with the boot server first
    D( printf("> identify\n"); )
    while (bootLogic.identify(99, (const uint8_t*) (result + 1))) {
        uint16_t swid = bootLogic.reply.h.swId;
        uint16_t size = bootLogic.reply.h.swSize;
        uint16_t crc = bootLogic.reply.h.swCrc;
        D( printf("  swid %u size %u crc %x\n", swid, size, crc); )
        
        // if current code matches size and crc, we're done
        uint16_t myCrc = Util::calculateCrc(CRC_INIT, (void*) PAGE_BASE, size);
        D( printf("  myCrc %x\n", myCrc); )
        if (crc == myCrc)
            break;

        // nope, we need to download the latest firmware
        D( printf("> fetchAll %u\n", swid); )
        bootLogic.fetchAll(swid);
    }

    // finally, prepare to jump to user code:
    //  * adjust dispatch vector base
    //  * reset stack
    //  * jump to reset vector entry

    D( printf("JUMP!\n"); )

    // see http://markdingst.blogspot.nl/
    //  2012/06/make-own-bootloader-for-arm-cortex-m3.html
    __ASM("ldr     r0, =0x1000\n" // TODO should be PAGE_BASE, how?
          "ldr     r1, =0xE000ED08\n"
          "str     r0, [r1]\n"
          "ldr     r1, [r0]\n"
          "mov     sp, r1\n"
          "ldr     r0, [r0, #4]\n"
          "bx      r0\n");
}