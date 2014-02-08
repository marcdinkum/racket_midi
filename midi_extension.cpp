/**********************************************************************
*          Copyright (c) 2014, Hogeschool voor de Kunsten Utrecht
*                      Hilversum, the Netherlands
*                          All rights reserved
***********************************************************************
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.
*  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************
*
*  File name     : midi_extension.cpp
*  System name   : racket_midi
* 
*  Description   : MIDI I/O module for Racket
*		   Needs Portmidi and Racket
*
*  Author        : Marc Groenewegen
*  E-mail        : marcg@dinkum.nl
*
**********************************************************************/

#include "escheme.h"
#include "midi_io.h"


/*
 * The following functions are exported to Scheme. NB: after initialising
 * you can't change input or output so set input/output device BEFORE
 * calling start-midi-io!
 *
 * (list-midi-devices)
 * (set-midi-input)
 * (set-midi-output)
 * (start-midi-io)
 * (stop-midi-io)
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


static Scheme_Object *startmidi(int argc, Scheme_Object **argv)
{
  int status=midi_io.initialise();
  if(status < 0){
    switch(status) {
      case ERROR_OPEN_INPUT: scheme_signal_error("Error opening input port");
      break;
      case ERROR_OPEN_OUTPUT: scheme_signal_error("Error opening output port");
      break;
      default: scheme_signal_error("Unknown error");
      break;
    } // switch
  }
  return scheme_void;
} // startmidi()


static Scheme_Object *stopmidi(int argc, Scheme_Object **argv)
{
  midi_io.finalise();
  return scheme_void;
} // stopmidi()


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

  // make sure we start with a clean slate
  midi_io.finalise();

  proc=scheme_make_prim_w_arity(list_devices,"list-midi-devices",0,0);
  scheme_add_global("list-midi-devices",proc,mod_env);
  proc=scheme_make_prim_w_arity(set_input_device,"set-midi-input",1,1);
  scheme_add_global("set-midi-input",proc,mod_env);
  proc=scheme_make_prim_w_arity(set_output_device,"set-midi-output",1,1);
  scheme_add_global("set-midi-output",proc,mod_env);
  proc=scheme_make_prim_w_arity(startmidi,"start-midi-io",0,0);
  scheme_add_global("start-midi-io",proc,mod_env);
  proc=scheme_make_prim_w_arity(stopmidi,"stop-midi-io",0,0);
  scheme_add_global("stop-midi-io",proc,mod_env);
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

