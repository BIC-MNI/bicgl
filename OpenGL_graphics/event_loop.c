 
#include  <internal_volume_io.h>
#include  <GS_graphics.h>

public  void  GS_set_update_function(
    void  (*func)( void ) )
{
    WS_set_update_function( func );
}

public  void  GS_event_loop( void )
{
    WS_event_loop();
}
