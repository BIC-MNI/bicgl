
#include  <internal_volume_io.h>
#include  <graphics.h>

private  Random_mask_type  random_order_masks[] = {
                                                 0x0,
                                                 0x0,
                                                 0x06,
                                                 0x0c,
                                                 0x14,
                                                 0x30,
                                                 0x60,
                                                 0xb8,
                                                 0x0110,
                                                 0x0240,
                                                 0x0500,
                                                 0x0ca0,
                                                 0x1b00,
                                                 0x3500,
                                                 0x6000,
                                                 0xb400,
                                                 0x00012000,
                                                 0x00020400,
                                                 0x00072000,
                                                 0x00090000,
                                                 0x00140000,
                                                 0x00300000,
                                                 0x00400000,
                                                 0x00d80000,
                                                 0x01200000,
                                                 0x03880000,
                                                 0x07200000,
                                                 0x09000000,
                                                 0x14000000,
                                                 0x32800000,
                                                 0x48000000,
                                                 0xa3000000
                                               };

public  Random_mask_type  get_random_order_mask(
    int  size )
{
    int  index;

    index = 0;

    while( size != 0 )
    {
        size >>= 1;
        ++index;
    }

    return( random_order_masks[index] );
}
