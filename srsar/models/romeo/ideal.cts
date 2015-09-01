/* Paramètres du modèle */
parameters 
        init_latency       = 40
    and processing_latency = 60 
    and buffer_period      = 34
    and firing_time        = 5
    and rearming_time      = 7
    and looping_time       = buffer_period
    and polling_time       = 1
    and driver_period      = 50
    and transmission_time  = 2
    and receiving_time     = 4


/* Initialisation de l'ensemble des variables */
initially 
    DEV_camera        := 0,
    DEV_camera_buffer := 0,
    DEV_racket        := 0,
    ARD_loop          := 0,
    ARD_handler       := 0,
    NXT_driver        := 0,
    NXT_driver_period := 0,
    NXT_buffer        := 0,
    NXT_boot          := 0,
    NXT_task          := 0

// ===================================

/* Traduction du processus DEV_camera */
transition Camera_Acquiring [0, init_latency]
  when DEV_camera  = 2
  do   DEV_camera := 3

transition Camera_Updating [buffer_period, buffer_period]
  when DEV_camera  = 4
  do   DEV_camera := 5

transition Camera_Looping [0, 0]
  when DEV_camera         = 5
  and  DEV_camera_buffer  = 0
  do   DEV_camera        := 4,
       DEV_camera_buffer := 1

/* Traduction du processus DEV_racket */
transition Racket_Firing [firing_time, firing_time]
  when DEV_racket  = 1
  do   DEV_racket := 2

/* Traduction du processus NXT_driver */
transition Driver_PreActivating [driver_period, driver_period]
  when NXT_driver_period  = 1
  do   NXT_driver_period := 2

transition Driver_Activating [0, 0]
  when NXT_driver_period  = 2
  and  NXT_driver         = 6
  do   NXT_driver_period := 0,
       NXT_driver        := 1

transition Driver_OverItsPeriod [1, inf[
  when NXT_driver_period  = 2
  do   DEV_camera        := -1,
       DEV_camera_buffer := -1,
       DEV_racket        := -1,
       ARD_loop          := -1,
       ARD_handler       := -1,
       NXT_driver        := -1,
       NXT_driver_period := -1,
       NXT_buffer        := -1,
       NXT_boot          := -1,
       NXT_task          := -1

/* Traduction du processus NXT_task */
transition Task_Processing [0, 0]
  when NXT_task  = 3
  do   NXT_task := 1

// ===================================

/* Synchro CMUcam */
transition CMUcam_begin
  when ARD_loop    = 0
  and  DEV_camera  = 0
  do   ARD_loop   := 1,
       DEV_camera := 1

transition CMUcam_trackColor
  when ARD_loop    = 1
  and  DEV_camera  = 1
  do   ARD_loop   := 2,
       DEV_camera := 2

transition CMUcam_putTypeTDataPacket [0, processing_latency]
  when DEV_camera         = 3
  and  NXT_boot           = 0
  do   DEV_camera        := 4,
       DEV_camera_buffer := 1,
       NXT_boot          := 1

transition CMUcam_getTypeTDataPacket_full [1, looping_time[
  when DEV_camera_buffer  = 1
  and  ARD_loop           = 2
  do   DEV_camera_buffer := 0

//transition CMUcam_getTypeTDataPacket_empty [1, looping_time[
//  when DEV_camera_buffer = 0
//  and (DEV_camera = 4 or DEV_camera = 5)
//  and  ARD_loop = 2

/* Synchro Driver */
transition Driver_Busy [0, 0]
  when NXT_driver         = 1
  and  NXT_driver_period  = 0
  and  ARD_handler        = 0
  do   NXT_driver        := 2,
       NXT_driver_period := 1,
       ARD_handler       := 1

transition Driver_Waiting [receiving_time, receiving_time]
  when NXT_driver   = 4
  and  ARD_handler  = 3
  do   NXT_driver  := 5,
       ARD_handler := 0

/* Synchro Wire */
transition Wire_onRequest [transmission_time, transmission_time]
  when NXT_driver   = 2
  and  ARD_handler  = 1
  do   NXT_driver  := 3,
       ARD_handler := 2

transition Wire_write [polling_time, polling_time]
  when ARD_handler  = 2
  and  NXT_driver   = 3
  do   ARD_handler := 3,
       NXT_driver  := 4

/* Synchro OSEK */
transition OSEK_StartOS [0, 0]
  when NXT_boot    = 1
  and  NXT_buffer  = 0
  and  NXT_driver  = 0
  and  NXT_task    = 0
  do   NXT_boot   := 2,
       NXT_buffer := 1,
       NXT_driver := 1,
       NXT_task   := 1

transition OSEK_SendMessage [0, 0]
  when NXT_driver  = 5
  and  NXT_buffer  = 1
  and  NXT_task    = 1
  do   NXT_driver := 6,
       NXT_task   := 2

transition OSEK_ReceiveMessage [0, 0]
  when NXT_task    = 2
  and  NXT_buffer  = 1
  do   NXT_task   := 3

/* Synchro Racket */
transition Racket_Shoot [0, 0]
  when NXT_task    = 3
  and  DEV_racket  = 0
  do   NXT_task   := 4,
       DEV_racket := 1

transition Racket_Rearmed [rearming_time, rearming_time]
  when DEV_racket  = 2
  and  NXT_task    = 4
  do   DEV_racket := 0,
       NXT_task   := 1

//===================================

// check[ip] AG(not(deadlock))
// check[ip] (not(NXT_driver = 1)) --> (NXT_driver = 1)
// check[ip] (NXT_boot = 2 and not(NXT_driver = 1)) -->[0, 25] (NXT_driver = 1)
