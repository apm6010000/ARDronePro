/**
 * @file main.c
 * @author sylvain.gaeremynck@parrot.com
 * @date 2009/07/01
 */
#include <ardrone_testing_tool.h>

//ARDroneLib
#include <ardrone_tool/ardrone_time.h>
#include <ardrone_tool/Navdata/ardrone_navdata_client.h>
#include <ardrone_tool/Control/ardrone_control.h>
#include <ardrone_tool/UI/ardrone_input.h>

//Common
#include <config.h>
#include <ardrone_api.h>

//VP_SDK
#include <ATcodec/ATcodec_api.h>
#include <VP_Os/vp_os_print.h>
#include <VP_Api/vp_api_thread_helper.h>
#include <VP_Os/vp_os_signal.h>

//Local project
#include <Video/video_stage.h>

static int32_t exit_ihm_program = 1;

/* Implementing Custom methods for the main function of an ARDrone application */

/* The delegate object calls this method during initialization of an ARDrone application */

////Control System (Designed by Pratik) --Start

DEFINE_THREAD_ROUTINE( thread1, data )
{
    int seq_no = 0,prev_start = 0;
    PRINT( "Initilizing Thread 1\n" );
    while(1)
    {
    vp_os_delay(100);

    vp_os_mutex_lock( &control_data_lock ); //Taking Control Mutex

    if( prev_start == 0 && control_data.start == 1 )
    {
        ardrone_tool_set_ui_pad_start( 1 );
        prev_start = 1;
    }
    else if( prev_start == 1 && control_data.start == 0 )
    {
        ardrone_tool_set_ui_pad_start( 0 );
        prev_start = 0;
    }
    ardrone_at_set_progress_cmd( ++seq_no,control_data.roll, control_data.pitch, control_data.gaz, control_data.yaw); //command to make drone move.
    vp_os_mutex_unlock( &control_data_lock );
    }
}

////Control System (Designed by Pratik) --END

C_RESULT ardrone_tool_init_custom(int argc, char **argv)
{
  /* Registering for a new device of game controller */

  /* Start all threads of your application */
  //control_data = (control_data_t *) malloc (sizeof(control_data_t);
  vp_os_mutex_init( &control_data_lock );
  START_THREAD( video_stage, NULL );
  START_THREAD( ardrone_control, NULL );
  START_THREAD( thread1, NULL );
  
  return C_OK;
}

/* The delegate object calls this method when the event loop exit */
C_RESULT ardrone_tool_shutdown_custom()  //Initiation Function
{
  /* Relinquish all threads of your application */
  JOIN_THREAD( video_stage );
  JOIN_THREAD( thread1 );
  ardrone_tool_set_ui_pad_start(0);

  /* Unregistering for the current device */

  return C_OK;
}

/* The event loop calls this method for the exit condition */
bool_t ardrone_tool_exit()
{
  return exit_ihm_program == 0;
}

C_RESULT signal_exit()
{
  exit_ihm_program = 0;

  return C_OK;
}

/* Implementing thread table in which you add routines of your application and those provided by the SDK */
BEGIN_THREAD_TABLE
  THREAD_TABLE_ENTRY( ardrone_control, 20 )
  THREAD_TABLE_ENTRY( navdata_update, 20 )
  THREAD_TABLE_ENTRY( video_stage, 20 )
  THREAD_TABLE_ENTRY( thread1, 20 )
END_THREAD_TABLE

