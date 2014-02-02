;**********************************************************************
;          Copyright (c) 2014, Hogeschool voor de Kunsten Utrecht
;                      Hilversum, the Netherlands
;                          All rights reserved
;**********************************************************************
;  This program is free software: you can redistribute it and/or modify
;  it under the terms of the GNU General Public License as published by
;  the Free Software Foundation, either version 3 of the License, or
;  (at your option) any later version.
;
;  This program is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License
;  along with this program.
;  If not, see <http://www.gnu.org/licenses/>.
;**********************************************************************
;
;  File name     : midi_init.rkt
;  System name   : racket_midi
; 
;  Description   : This file is meant to get you started with the Racket
;		   MIDI I/O module
;
;  Decide whether to use DrRacket or the CLI. The CLI works nicer for
;  repeating commands, copy-pasting code and developing interactively
;  but lacks the debugging features of DrRacket
;
;  Author        : Marc Groenewegen
;  E-mail        : marcg@dinkum.nl
;
;*********************************************************************/

; DrRacket
#lang racket

(require "midi_extension.rkt")

(list-midi-devices)

; DrRacket
(display "Input: ")
(set-midi-input (string->number (read-line)))
(display "Output: ")
(set-midi-output (string->number (read-line)))

; racket CLI
; (display "Input: ")
; (set-midi-input (read))

; racket CLI
; (display "Output: ")
; (set-midi-output (read))


; Start the MIDI engine. Do this AFTER selecting in- and output!
(start-midi-io)

; midi events are lists '(cmd channel data1 data2)
; if cmd<128 this means no event was read


; calling read-loop will translate all incoming events to note-ons for
;  testing and it's quite fun too
; Sending a pitch bend event breaks the loop
;
(define (read-loop)
  (let loop ((event (read-midi-event)))
    (when (>= (car event) 128) ; 128 means status bit set, indicating something was received
      (begin
        (display (format "~a ~a ~a ~a"
	  (car event) (cadr event) (caddr event) (cadddr event)))
        (displayln (caddr event))
        (note-on 0 (caddr event) 100)))
    (sleep 0.01)
    ; break only upon receiving a pitch bend event
    (when (or (< (car event) 128) (not (= (car event) 224)))
      (loop (read-midi-event)))))

; convenience function
(define (all-notes-off [channel 0] [from 0] [to 127])
  (define (helper n)
    (if (>= n to) (void)
        (begin
          (note-off channel n 127)
          (helper (+ n 1)))))
  (helper from))


; convenience function
(define (flush-input-queue)
  (let loop ((event (read-midi-event)))
    (when (>= (car event) 128) (loop (read-midi-event)))))


; disconnect from the MIDI engine, freeing connected synths and opening
;  opening the possibility for re-routing to start over
(define (midi-disconnect)
  (stop-midi-io))

