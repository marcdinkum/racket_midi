#include "escheme.h"
#include "midi_io.h"


/*
 * The following functions are exported to Scheme. NB: after initialising
 * you can't change input or output so set input/output device BEFORE
 * calling init-midi-io!
 *
 * (list-devices)
 * (set-input-device)
 * (set-output-device)
 * (init-midi-io)
 * (note-on)
 * (note-off)
 * (read-midi-event)
 */

MIDI_io midi_io;


static Scheme_Object *list_devices(int argc, Scheme_Object **argv)
{
const PmDeviceInfo *info; // portMidi device info
Scheme_Object *output_list;

  /*
   * Use PortMidi info to build an array of device info objects
   */
  Scheme_Object *device_info[Pm_CountDevices()];

  /* device info object with details about the device:
   *  int index
   *  string direction -->  IN or OUT
   *  string name
   */
  Scheme_Object *device_info_detail[Pm_CountDevices()][3];

  // Build a list of lists
  for(int d=0;d<Pm_CountDevices();d++)
  {
    device_info_detail[d][0]=scheme_make_integer(d);
    info = Pm_GetDeviceInfo(d);

    if(info->input > 0)
      device_info_detail[d][1]=scheme_make_utf8_string("IN");
    else if(info->output > 0)
      device_info_detail[d][1]=scheme_make_utf8_string("OUT");
    else
      device_info_detail[d][1]=scheme_make_utf8_string("-");

    device_info_detail[d][2]=scheme_make_utf8_string(info->name);

    device_info[d]=scheme_build_list(3,device_info_detail[d]);
  } // for

  output_list=scheme_build_list(Pm_CountDevices(),device_info);

  return output_list;
} // list_devices()


static Scheme_Object *set_input_device(int argc, Scheme_Object **argv)
{
intptr_t device;

  scheme_get_int_val(argv[0],&device);
  midi_io.set_input_device((int)device);
  return scheme_void;
} // set_input_device()


static Scheme_Object *set_output_device(int argc, Scheme_Object **argv)
{
intptr_t device;

  scheme_get_int_val(argv[0],&device);
  midi_io.set_output_device((int)device);
  return scheme_void;
} // set_output_device()


static Scheme_Object *init(int argc, Scheme_Object **argv)
{
  midi_io.initialise();
  return scheme_void;
} // init()


static Scheme_Object *note_on(int argc, Scheme_Object **argv)
{
intptr_t channel,note,velocity;
PmEvent event;

  scheme_get_int_val(argv[0],&channel);
  scheme_get_int_val(argv[1],&note);
  scheme_get_int_val(argv[2],&velocity);

  /*
    #define Pm_Message(status, data1, data2) \
              ((((data2) << 16) & 0xFF0000) | \
               (((data1) << 8) & 0xFF00) | \
               ((status) & 0xFF))
  */
  event.message=Pm_Message(0x90+(int)channel,(int)note,(int)velocity);
  event.timestamp=0;
  midi_io.write_event(&event);

  return scheme_void;
} // note_on()


static Scheme_Object *note_off(int argc, Scheme_Object **argv)
{
intptr_t channel,note,velocity;
PmEvent event;

  scheme_get_int_val(argv[0],&channel);
  scheme_get_int_val(argv[1],&note);
  scheme_get_int_val(argv[2],&velocity);

  event.message=Pm_Message(0x80+(unsigned char)channel,
                    (unsigned char)note,
                    (unsigned char)velocity);
  event.timestamp=0;
  midi_io.write_event(&event);

  return scheme_void;
} // note_off()


static Scheme_Object *read_event(int argc, Scheme_Object **argv)
{
PmEvent event;
bool event_read;
unsigned char cmd,channel,data1,data2;
Scheme_Object *resultvector[4];

  event_read=midi_io.read_event(event); // event is passed by reference
  if(event_read){
    cmd=Pm_MessageStatus(event.message)&0xf0;
    channel=Pm_MessageStatus(event.message)&0xf;
    data1=Pm_MessageData1(event.message);
    data2=Pm_MessageData2(event.message);
    resultvector[0]=scheme_make_integer((int)cmd);
    resultvector[1]=scheme_make_integer((int)channel);
    resultvector[2]=scheme_make_integer((int)data1);
    resultvector[3]=scheme_make_integer((int)data2);
    return scheme_build_list(4,resultvector);
  } // if event read
  else {
    resultvector[0]=resultvector[1]=resultvector[2]=resultvector[3]=scheme_make_integer(0);
    return scheme_build_list(4,resultvector);
  }
} // read_event()



Scheme_Object *scheme_reload(Scheme_Env *env)
{
Scheme_Object *proc;
Scheme_Env *mod_env;
mod_env = scheme_primitive_module(scheme_intern_symbol("midi_extension"),env);

  proc=scheme_make_prim_w_arity(list_devices,"list-devices",0,0);
  scheme_add_global("list-devices",proc,mod_env);
  proc=scheme_make_prim_w_arity(set_input_device,"set-input-device",1,1);
  scheme_add_global("set-input-device",proc,mod_env);
  proc=scheme_make_prim_w_arity(set_output_device,"set-output-device",1,1);
  scheme_add_global("set-output-device",proc,mod_env);
  proc=scheme_make_prim_w_arity(init,"init",0,0);
  scheme_add_global("init-midi-io",proc,mod_env);
  proc=scheme_make_prim_w_arity(note_on,"note-on",3,3);
  scheme_add_global("note-on",proc,mod_env);
  proc=scheme_make_prim_w_arity(note_off,"note-off",3,3);
  scheme_add_global("note-off",proc,mod_env);
  proc=scheme_make_prim_w_arity(read_event,"read-midi-event",0,0);
  scheme_add_global("read-midi-event",proc,mod_env);

  scheme_finish_primitive_module(mod_env);

  return scheme_void;
} // scheme_reload()


Scheme_Object *scheme_initialize(Scheme_Env *env)
{
  return scheme_reload(env);
} // scheme_initialize()


// called when the extension is loaded to satisfy a require declaration
Scheme_Object *scheme_module_name()
{
  return scheme_intern_symbol("midi_extension");
} // scheme_module_name()

