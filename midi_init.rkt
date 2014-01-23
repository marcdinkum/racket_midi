; This file is meant to get you started with the Racket MIDI I/O module
;
; Decide whether to use DrRacket or the CLI. The CLI works nicer for
;  repeating commands, copy-pasting code and developing interactively
;  but lacks the debugging features of DrRacket

; DrRacket
#lang racket

(require "midi_extension.rkt")

(list-devices)

; DrRacket
(display "Input: ")
(set-input-device (string->number (read-line)))
(display "Output: ")
(set-output-device (string->number (read-line)))

; racket CLI
; (display "Input: ")
; (set-input-device (read))

; racket CLI
; (display "Output: ")
; (set-output-device (read))


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

